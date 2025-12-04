package com.team42.lightapp

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.os.Build
import android.os.Handler
import android.os.Bundle
import android.os.Looper
import android.os.Message
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.core.content.ContextCompat.getSystemService
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

const val MESSAGE_READ: Int = 0
const val MESSAGE_WRITE: Int = 1
const val MESSAGE_TOAST: Int = 2


class HardwareSystem(
    private val context: Context
)
{
    // Public Hardware Information
    var sectionCount = 0
    val ledList : MutableList<LEDInfo> = mutableListOf()
    val externalModuleMap : MutableMap<Int, ExternalModule> = mutableMapOf()
    var state : HardwareState = HardwareState.DISCONNECTED

    // Finds the required bluetooth connection from the list of paired devices, attempts to connect
    @RequiresApi(Build.VERSION_CODES.S)
    @RequiresPermission(allOf = [Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT])
    fun connectToPairedDevice() : Boolean
    {
        val bluetoothManager: BluetoothManager? = getSystemService(context, BluetoothManager::class.java)

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

            if(deviceName == "ESP_SPP_ACCEPTOR")
            {
                bluetoothDevice = device
            }
        }


        // Get the socket
        bluetoothSocket = bluetoothDevice?.createRfcommSocketToServiceRecord(sppUUID) ?: return false

        // Attempt to connect
        bluetoothAdapter.cancelDiscovery()
        try
        {
            bluetoothSocket?.connect() ?: return false
        }
        catch (e : IOException)
        {
            return false
        }

        // Device connected, start bluetooth thread
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

        state = HardwareState.CONNECTED

        // Previously initialized, maintain these values
        if(sectionCount != 0)
        {
            state = HardwareState.INITIALIZED
        }
        return true
    }

    fun closeConnection() : Boolean
    {
        try {
            bluetoothSocket?.close()
        } catch (e: IOException) {
            return false
            //Log.e(TAG, "Could not close the connect socket", e)
        }
        state = HardwareState.DISCONNECTED
        return true
    }

    // Functions to communicate with the microcontroller
    // All functions should wait until the state is initialized (Except uC_GetInfo)
    // All will throw an error if the BT thread is not started
    fun uC_SendSession(session : LightSession)
    {
        var message = "SendSession,${session.blocks.count()},${sectionCount}"

        for(block in session.blocks)
        {
            message += ",\n"
            message += "${block.timeStamp}"
            for(i in 0 until sectionCount)
            {
                message += ",${block.lights[i].brightness},${block.lights[i].frequency}"
            }
        }

        btThread!!.write(message.toByteArray())
    }
    fun uC_StopAll()
    {
        btThread!!.write("StopAll".toByteArray())
    }
    fun uC_SetSection(index : Int, source : LightSource)
    {
        if(index < sectionCount) throw Exception("Index cannot be greater than section count")

        btThread!!.write("SetSection,${index},${source.brightness},${source.frequency}".toByteArray())
    }
    fun uC_GetInfo()
    {
        btThread!!.write("GetInfo".toByteArray())
    }
    fun uC_GetUpdates(index : Int, receiveUpdates : Boolean, frequency: Int)
    {

        btThread!!.write("GetUpdates,${index},${receiveUpdates},${frequency}".toByteArray())
    }

    // State of the Hardware system
    enum class HardwareState
    {
        DISCONNECTED,
        CONNECTED,
        INITIALIZED,
    }

    // Bluetooth connection
    private var bluetoothDevice : BluetoothDevice? = null
    private var bluetoothSocket: BluetoothSocket? = null
    private var btThread : ConnectedThread? = null
    private val sppUUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    // Thread to handle bluetooth input / output streams
    private inner class ConnectedThread : Thread() {

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

                // Add null terminator to byte array
                mmOutStream.write(bytes.plus(0.toByte()))
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

    // Bluetooth socket connection uses another thread (ConnectedThread class)
    // This handler operates on the main thread and interfaces with the microcontroller functions
    private val handler = object : Handler(Looper.getMainLooper()) {
        override fun handleMessage(msg: Message) {
            when (msg.what) {
                MESSAGE_READ -> {
                    val readBytes = msg.obj as ByteArray
                    val readMessage = String(readBytes, 0, msg.arg1)
                    parseReceivedMessage(readMessage)
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
    private fun parseReceivedMessage(message : String)
    {
        val split = message.split(",")


        when(split[0])
        {
            "SetInfo" -> {
                try {
                    val ledCount = split[1].toInt()
                    sectionCount = split[2].toInt()

                    // Reset old values
                    ledList.clear()
                    externalModuleMap.clear()


                    // Add all LEDs
                    for(i in 0 until ledCount)
                    {
                        // Check invalid parameter count
                        if(5 + 3*i >= split.count())
                        {
                            Toast.makeText(context, "Invalid Parameter Count", Toast.LENGTH_SHORT).show()
                            return
                        }

                        // Check invalid section
                        val section = split[5 + 3*i].toInt()
                        if(section >= sectionCount)
                        {
                            Toast.makeText(context, "Invalid Section Number: $section\nMax: ${sectionCount-1}", Toast.LENGTH_SHORT).show()
                        }

                        ledList.add(LEDInfo(split[3 + 3*i].toInt(), split[4 + 3*i].toInt(), section))
                    }

                    // Check for addition inputs
                    var currentIndex = 3 * (ledCount + 1)
                    while(currentIndex + 3 < split.count())
                    {
                        val isInput : Boolean = split[currentIndex + 1] == "Input"
                        externalModuleMap[split[currentIndex].toInt()] =
                            ExternalModule(isInput, split[currentIndex + 2], split[currentIndex + 3])
                        currentIndex += 4
                    }

                    state = HardwareState.INITIALIZED
                }
                catch (e : NumberFormatException) {
                    Toast.makeText(context, "Invalid Parameters for SetInfo", Toast.LENGTH_SHORT).show()
                }
            }
            "SendUpdate" -> {
                try {
                    val eID = split[1].toInt()
                    val value = split[2].toDouble()
                    externalModuleMap[eID]!!.value = value
                }
                catch (e : NumberFormatException)
                {
                    Toast.makeText(context, "Invalid Parameters for SendUpdate", Toast.LENGTH_SHORT).show()
                }
                catch (e : NullPointerException)
                {
                    Toast.makeText(context, "External Device with ID: ${split[1].toInt()} does not exist", Toast.LENGTH_SHORT).show()
                }
            }
            "SendError" -> {
                Toast.makeText(context, "Error: ${split[1]}", Toast.LENGTH_SHORT).show()
            }
            else -> Toast.makeText(context, "Unrecognized Command: ${split[0]}", Toast.LENGTH_SHORT).show()
        }
    }


    // Test Function, delete later
    fun parseTest(message: String)
    {
        parseReceivedMessage(message)
    }

}

class LightSession(
    val name : String,
    val blocks : MutableList<SessionBlock> = mutableListOf()
)

class SessionBlock(
    val lights  : MutableList<LightSource> = mutableListOf(),
    val timeStamp  : Double = 0.0,
)
{
    fun setSection(index : Int, source : LightSource)
    {
        if(index < lights.count())
        {
            lights[index] = source
        }
        else
        {
            // Out of range
        }
    }
}

class LightSource(
    val brightness  : Double = 0.0,
    val frequency   : Double = 0.0
)

class LEDInfo(
    val x : Int = 0,
    val y : Int = 0,
    val section : Int = 0
)

class ExternalModule(
    val isInput : Boolean = false,
    val name: String = "",
    val description : String = "",
    var value: Double = 0.0
)

