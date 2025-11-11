package com.team42.lightapp.ui.debug


import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.companion.BluetoothDeviceFilter
import android.companion.AssociationRequest
import android.companion.CompanionDeviceManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.getSystemService
import androidx.core.content.ContextCompat.registerReceiver
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager
import java.io.IOException
import java.util.UUID


class DebugFragment : Fragment() {

    companion object {
        fun newInstance() = DebugFragment()
    }

    private val viewModel: DebugViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // TODO: Use the ViewModel
        // Setup sessionmanager
        SessionManager.getFolder(requireContext())
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val rootView = inflater.inflate(R.layout.fragment_debug, container, false)

        // Find the button by its ID
        val saveButton: Button = rootView.findViewById(R.id.savebutton)
        val bluetoothButton : Button = rootView.findViewById(R.id.bluetoothbutton)

        // Set a click listener on the button
        saveButton.setOnClickListener {
            // Handle the button click event here
            viewModel.saveSessionTest()
        }

        bluetoothButton.setOnClickListener{
            bluetoothTest()
        }


        return rootView
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    @RequiresApi(Build.VERSION_CODES.S)
    @RequiresPermission(allOf = [Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT])
    fun bluetoothTest()
    {
        ActivityCompat.requestPermissions(
            context as Activity,
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT,
                Manifest.permission.ACCESS_FINE_LOCATION
            ),
            1001
        )

        val bluetoothManager: BluetoothManager? =
            context?.let { getSystemService(it, BluetoothManager::class.java) }
        val bluetoothAdapter: BluetoothAdapter? = bluetoothManager?.getAdapter()

        if (bluetoothAdapter == null) {
            // Device doesn't support Bluetooth
            return
        }

        if (bluetoothAdapter?.isEnabled == false) {
            // Not on. Prompt to turn on (not now)
            return
        }

        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter?.bondedDevices
        pairedDevices?.forEach { device ->
            val deviceName = device.name
            val deviceHardwareAddress = device.address // MAC address
        }

        if (pairedDevices != null) {
            for(dev in pairedDevices)
            {
                val x = dev.name
                if(dev.name == "ESP_SPP_ACCEPTOR")
                {
                    // Do stuff
                }
            }
        }
    }
}
