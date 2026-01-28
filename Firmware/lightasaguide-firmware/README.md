| Supported Targets | ESP32 |
| ----------------- | ----- |

## ECE 4805 S26-42: Light As A Guide
## ESP32 Firmware

This program is designed to wirelessly receive and parse sequences to drive an LED array with 4 independent groups. It uses the ESP-IDF library and FreeRTOS framework. The Bluetooth API is configured to receive data serially over Bluetooth Classic via the SPP protocol. The microcontroller must be connected to an Android device with the custom-made app for remote control (SPP is not supported on iOS devices).

## How to use

### Hardware Required

This program is designed to run on commonly available ESP32 development board, e.g. ESP32-DevKitC.

### Configure the project

1. Open the project configuration menu:

```
idf.py menuconfig
```

2. Enable the SPP functionality by choosing the path as following:

`Component config --> Bluetooth --> Bluedroid Options --> SPP`

3. If you want to limit the number of connected devices, please make sure set the `BT/BLE MAX ACL CONNECTIONS` and `BR/EDR ACL Max Connections` with same value you want.

`Component config --> Bluetooth --> Bluedroid Options --> BT/BLE MAX ACL CONNECTIONS(1~7)`
and
`Component config --> Bluetooth --> Bluetooth --> Bluetooth controller --> BR/EDR ACL Max Connections`


4. SSP is enabled as default in this example. If you prefer the legacy pairing, you shall disable it in the following path.

`SPP Example Configuration --> Secure Simple Pair`.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Description

After the program starts, the example will start an SPP acceptor. The example will calculate the data rate or just print the received data after the SPP connection is established. You can connect to the server and send data with another ESP32 development board, Andriod phone or computer which performs as the SPP initiator.