package com.team42.lightapp

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.hardware.lights.Light
import android.os.Build
import android.os.Handler
import android.os.Bundle
import android.os.Looper
import android.os.Message
import android.se.omapi.Session
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat.getSystemService
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class LightSession(
    val name : String,
    val blocks : MutableList<SessionBlock> = mutableListOf<SessionBlock>()
)
{}

const val MESSAGE_READ: Int = 0
const val MESSAGE_WRITE: Int = 1
const val MESSAGE_TOAST: Int = 2

class HardwareSystem(
    private val context: Context
)
{
    // Hardware information
    var sectionCount = 0
    private var ledList : List<LEDInfo> = emptyList()
    fun getLED(ledID : Int) : LEDInfo {return LEDInfo()}

    // Call getInfo, set hardware info
    // Todo
    //init{}

    // Bluetooth connection
    private var bluetoothDevice : BluetoothDevice? = null
    private var bluetoothSocket: BluetoothSocket? = null
    private var btThread : ConnectedThread? = null
    private val sppUUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    // Bluetooth socket connection uses another thread (ConnectedThread class)
    // This handler operates on the main thread and interfaces with the microcontroller functions
    //private val handler: Handler = Handler()
    private val handler = object : Handler(Looper.getMainLooper()) {
        override fun handleMessage(msg: Message) {
            when (msg.what) {
                MESSAGE_READ -> {
                    val readBytes = msg.obj as ByteArray
                    val readMessage = String(readBytes, 0, msg.arg1)
                    // Update UI with received message
                    Toast.makeText(context, "Received: $readMessage", Toast.LENGTH_SHORT).show()
                }
                MESSAGE_WRITE -> {
                    // Message successfully sent
                    Toast.makeText(context, "Message sent!", Toast.LENGTH_SHORT).show()
                }
                MESSAGE_TOAST -> {
                    val text = msg.data.getString("toast")
                    Toast.makeText(context, text, Toast.LENGTH_SHORT).show()
                }
            }
        }
    }

    // Finds the required bluetooth connection from the list of paired devices.
    @RequiresApi(Build.VERSION_CODES.S)
    @RequiresPermission(allOf = [Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT])
    fun connectToPairedDevice() : Boolean
    {
        val bluetoothManager: BluetoothManager? =
            context.let { getSystemService(it, BluetoothManager::class.java) }

        // If adapter is null, return false
        val bluetoothAdapter: BluetoothAdapter = bluetoothManager?.adapter ?: return false

        if (!bluetoothAdapter.isEnabled) {
            // Not on. Prompt to turn on (not now)
            return false
        }

        // Find the list of paired devices
        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter.bondedDevices
        pairedDevices?.forEach { device ->
            val deviceName = device.name
            val deviceHardwareAddress = device.address // MAC address
        }

        // Find the specific device
        if (pairedDevices != null) {
            for(dev in pairedDevices)
            {
                if(dev.name == "ESP_SPP_ACCEPTOR")
                {
                    bluetoothDevice = dev
                }
            }
        }

        // Get the socket
        bluetoothSocket = bluetoothDevice?.createRfcommSocketToServiceRecord(sppUUID)

        // Attempt to connect
        bluetoothAdapter.cancelDiscovery()
        try
        {
            bluetoothSocket?.connect() ?: return false
        }
        catch (e : java.io.IOException)
        {
            return false
        }

        return true
    }

    // Start the thread
    fun startBTThread() : Boolean
    {
        try
        {
            if(!bluetoothSocket!!.isConnected)
            {
                // Not connected
                return false
            }

            btThread = ConnectedThread()
            btThread?.start() ?: return false

        }
        catch (e : NullPointerException)
        {
            return false
        }

        return true
    }


    // Thread to handle bluetooth input / output streams
    private inner class ConnectedThread() : Thread() {


        private val mmInStream: InputStream = bluetoothSocket?.inputStream ?:
            throw NullPointerException("No bluetooth socket")

        private val mmOutStream: OutputStream = bluetoothSocket?.outputStream ?:
            throw NullPointerException("No bluetooth socket")

        private val mmBuffer: ByteArray = ByteArray(1024) // mmBuffer store for the stream

        override fun run() {

            var numBytes: Int // bytes returned from read()

            // Keep listening to the InputStream until an exception occurs.
            while (true) {
                // Read from the InputStream.
                numBytes = try {
                    mmInStream.read(mmBuffer)
                } catch (e: IOException) {
                    val writeErrorMsg = handler.obtainMessage(MESSAGE_TOAST)
                    val bundle = Bundle().apply {
                        putString("toast", "Device Disconnected")
                    }
                    writeErrorMsg.data = bundle
                    handler.sendMessage(writeErrorMsg)
                    break
                }

                // Send the obtained bytes to the UI activity.
                val readMsg = handler.obtainMessage(
                    MESSAGE_READ, numBytes, -1,
                    mmBuffer)
                readMsg.sendToTarget()
            }
        }

        // Call this from the main activity to send data to the remote device.
        fun write(bytes: ByteArray) {
            try {
                mmOutStream.write(bytes)
            } catch (e: IOException) {
                //Log.e(TAG, "Error occurred when sending data", e)

                // Send a failure message back to the activity.
                val writeErrorMsg = handler.obtainMessage(MESSAGE_TOAST)
                val bundle = Bundle().apply {
                    putString("toast", "Couldn't send data to the other device")
                }
                writeErrorMsg.data = bundle
                handler.sendMessage(writeErrorMsg)
                return
            }

            // Share the sent message with the UI activity.
            val writtenMsg = handler.obtainMessage(
                MESSAGE_WRITE, -1, -1, mmBuffer)
            writtenMsg.sendToTarget()
        }
    }

    fun closeConnection() : Boolean
    {
        try {
            bluetoothSocket?.close()
        } catch (e: IOException) {
            return false
            //Log.e(TAG, "Could not close the connect socket", e)
        }
        return true
    }

    fun sendTest() : Boolean
    {
        val bytes : ByteArray = "Test".toByteArray()
        btThread?.write(bytes) ?: return false
        return true
    }



    // Functions to communicate with the microcontroller
    fun uC_SendSession(session : LightSession) {}
    fun uC_StopSession(){}
    fun uC_SetSection(index : Int, source : LightSource){}
    fun uC_GetInfo(){}
    fun uC_GetUpdates(index : Int, receiveUpdates : Boolean, frequency: Int){}
}

class SessionBlock
(
    val lights  : List<LightSource> = emptyList<LightSource>(),
    val timeStamp  : Double = 0.0,
){
    fun setSection(index : Int, source : LightSource){}

}

class LightSource
(
    val brightness  : Double = 0.0,
    val frequency   : Double = 0.0
){}

class LEDInfo(
    val id : Int = 0,
    val x : Int = 0,
    val y : Int = 0,
    val section : Int = 0
) {}

