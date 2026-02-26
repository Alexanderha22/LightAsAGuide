#include "bluedroid_spp.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include "parse_data_task.h"

#include "led.h"

#include "time.h"
#include "sys/time.h"

#define SPP_TAG "SPP_ACCEPTOR"
#define SPP_SERVER_NAME "SPP_SERVER"

static const char local_device_name[] = CONFIG_EXAMPLE_LOCAL_DEVICE_NAME;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

static struct timeval time_old;

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static char *bda2str(uint8_t * bda, char *str, size_t size);


static RingbufHandle_t rx_rb = NULL;

static esp_spp_cb_param_t *global_param;
/*
 *   initializing bluetooth controller for use
 *   steps:
 *      1. free up memory from any unused bluetooth (optional)
 *      2. enable bluetooth hardware
 *      3. enable and configure bluedroid
*/ 
void bluetooth_init(RingbufHandle_t rb) {
    rx_rb = rb;

    char bda_str[18] = {0};
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // we are only using bluetooth classic
    // release unused ble memory from controller (disables ble mode)
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    // initialize the controller with default settings (configurable via menuconfig)
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // turn on the controller in bluetooth classic mode
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // default bluedroid settings:
    // - enable simple secure pairing
    // - disable secure connection host support
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    // sdkconfig override:
#if (CONFIG_EXAMPLE_SSP_ENABLED == false)
    bluedroid_cfg.ssp_en = false;
#endif

    // init and enable bluedroid api with config
    if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bluedroid_enable()) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // register callback function for bluetooth gap events 
    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s gap register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // resgiter callback function for bluetooth spp events
    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // vfs to store large strings ?
    esp_spp_cfg_t bt_spp_cfg = {
        .mode = esp_spp_mode,
        .enable_l2cap_ertm = esp_spp_enable_l2cap_ertm,
        .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
    };

    // bluetooth spp api
    if ((ret = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) { // stop at failure
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // menuconfig override to enable ssp (true)
#if (CONFIG_EXAMPLE_SSP_ENABLED == true)
    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /* legacy pairing : unused, not recommended
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    ESP_LOGI(SPP_TAG, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
}

bool bt_write(char *str, int len) {
    if (global_param->write.cong) { 
        ESP_LOGI(SPP_TAG, "Congested");
        return false;
    }
    ESP_LOGI(SPP_TAG, "Contents: %s", str);
    esp_spp_write(global_param->write.handle, len, (uint8_t *)str);
    return true;
}

// callback function to handle all spp events
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    global_param = param;
    char bda_str[18] = {0};

    switch (event) {

    case ESP_SPP_INIT_EVT: // start the spp server after initialization
        if (param->init.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
            esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
        } else {
            // todo error handling
            ESP_LOGE(SPP_TAG, "ESP_SPP_INIT_EVT status:%d", param->init.status);
        }
        break;
    
    case ESP_SPP_START_EVT: // server is ready to connect
        if (param->start.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT handle:%"PRIu32" sec_id:%d scn:%d", param->start.handle, param->start.sec_id,
                     param->start.scn);
            // set name and make discoverable
            esp_bt_gap_set_device_name(local_device_name);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            // todo: queue "discoverable" light sequence (fading in and out?)
        } else { 
            // todo error handling
            ESP_LOGE(SPP_TAG, "ESP_SPP_START_EVT status:%d", param->start.status);
        }
        break;

    case ESP_SPP_SRV_OPEN_EVT: // client connection successful
        // log for debug
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT status:%d handle:%"PRIu32", rem_bda:[%s]", param->srv_open.status,
                 param->srv_open.handle, bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
        gettimeofday(&time_old, NULL);
        ESP_LOGI(SPP_TAG, "SPP Server connection opened, handle: %d, rem_bda: %s",
                    param->srv_open.handle, bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
        ESP_LOGI(SPP_TAG, "Client Address: %s", bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
        ESP_LOGI(SPP_TAG, "Ready to receive data.");
        
        // Example: Send a welcome 
        const char *welcome_msg = "SPP connection established successfully\r\n";
        esp_spp_write(param->srv_open.handle, strlen(welcome_msg), (uint8_t *)welcome_msg);

        // todo: queue "successful connection" light sequence (blink twice?)

        break;

    case ESP_SPP_DATA_IND_EVT: // data is received
        // log for debug
        ESP_LOGI(SPP_TAG, "Received message with length %d", param->data_ind.len);

        const char *msg = "Data received\r\n";
        esp_spp_write(param->data_ind.handle, strlen(msg), (uint8_t *)msg);
        ESP_LOGW(SPP_TAG, "data sent to ring buffer: %s", param->data_ind.data);
        // defer processing to data handler task w/ ring buffer
        BaseType_t res = xRingbufferSend(rx_rb, param->data_ind.data, param->data_ind.len, pdMS_TO_TICKS( 10 ));
        if (res != pdTRUE) {
            ESP_LOGE(SPP_TAG, "Failed to send item \n");
        } else {
            ESP_LOGW(SPP_TAG, "Sent data to ring buffer successfully\r\n");
        }
        break;
    case ESP_SPP_CONG_EVT: // bluetooth stack is backed up (error)
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;

    case ESP_SPP_WRITE_EVT:
        // This event confirms that data passed to esp_spp_write has been sent to the lower layer.
        // It doesn't mean the remote device has received it yet.
        if (param->write.status == ESP_SPP_SUCCESS) {
                ESP_LOGI(SPP_TAG, "SPP Write successful, handle: %d, len: %d, cong: %s,",
                        param->write.handle, param->write.len, param->write.cong ? "congested" : "not congested");
            if (param->write.cong) {
                ESP_LOGW(SPP_TAG, "SPP link congested after write. Consider pausing further writes.");
            }
        } else {
            ESP_LOGE(SPP_TAG, "SPP Write failed, status: %d, handle: %d", param->write.status, param->write.handle);
        }
        break;
        
    case ESP_SPP_CLOSE_EVT: // connection ended
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT status:%d handle:%"PRIu32" close_by_remote:%d", param->close.status,
                 param->close.handle, param->close.async);\
        // if commands are in progress (queue is not empty) prepare for reconnection

            turn_off_leds();

        // else back to idle state

        break;

    default:
        break;
    }
}

//  callback function to handle all bluetooth connection events
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(SPP_TAG, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                     bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }

    // legacy pairing (not used)
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }
// secure simple pairing protocol
#if (CONFIG_EXAMPLE_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %06"PRIu32, param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%06"PRIu32, param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                 bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
        break;

    default: {
        ESP_LOGI(SPP_TAG, "event: %d", event);
        break;
    }
    }
    return;
}


// for debug log
static char *bda2str(uint8_t * bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}