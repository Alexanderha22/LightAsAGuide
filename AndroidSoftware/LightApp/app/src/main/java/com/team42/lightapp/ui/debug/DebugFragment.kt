package com.team42.lightapp.ui.debug


import android.Manifest
import android.annotation.SuppressLint
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
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.getSystemService
import androidx.core.content.ContextCompat.registerReceiver
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager
import java.io.IOException
import java.util.UUID


class DebugFragment : Fragment() {

    companion object {
        fun newInstance() = DebugFragment()
    }

    private val viewModel: DebugViewModel by viewModels()
    private var hs : HardwareSystem? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // TODO: Use the ViewModel
        // Setup sessionManager
        SessionManager.getFolder(requireContext())
        hs = HardwareSystem(requireContext())

    }

    @RequiresApi(Build.VERSION_CODES.S)
    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val rootView = inflater.inflate(R.layout.fragment_debug, container, false)

        // Find the button by its ID
        val saveButton: Button = rootView.findViewById(R.id.savebutton)
        val btConnectButton : Button = rootView.findViewById(R.id.btConnect)
        val btStartButton : Button = rootView.findViewById(R.id.btStart)
        val btSendButton : Button = rootView.findViewById(R.id.btSend)
        val permissionButton : Button = rootView.findViewById(R.id.getPermission)

        // Set a click listener on the button
        saveButton.setOnClickListener {
            // Handle the button click event here
            viewModel.saveSessionTest(hs!!)
        }

        // Get permission
        permissionButton.setOnClickListener{
            if (ActivityCompat.checkSelfPermission(
                    requireContext(),
                    Manifest.permission.BLUETOOTH_SCAN
                ) != PackageManager.PERMISSION_GRANTED || ActivityCompat.checkSelfPermission(
                    requireContext(),
                    Manifest.permission.BLUETOOTH_CONNECT
                ) != PackageManager.PERMISSION_GRANTED
            )
            {
                // Request
                ActivityCompat.requestPermissions(
                    context as Activity,
                    arrayOf(
                        Manifest.permission.BLUETOOTH_SCAN,
                        Manifest.permission.BLUETOOTH_CONNECT,
                        Manifest.permission.ACCESS_FINE_LOCATION
                    ),
                    1001
                )
            }
            else
            {
                Toast.makeText(context, "Permissions already Granted", Toast.LENGTH_SHORT).show()
            }
        }

        // Need to check permission everywhere
        btConnectButton.setOnClickListener {
            if (ActivityCompat.checkSelfPermission(
                    requireContext(),
                    Manifest.permission.BLUETOOTH_SCAN
                ) != PackageManager.PERMISSION_GRANTED || ActivityCompat.checkSelfPermission(
                    requireContext(),
                    Manifest.permission.BLUETOOTH_CONNECT
                ) != PackageManager.PERMISSION_GRANTED
            )
            {
                Toast.makeText(context, "Need Permission", Toast.LENGTH_SHORT).show()
            }
            else
            {
                if(hs!!.connectToPairedDevice())
                {
                    Toast.makeText(context, "Connected", Toast.LENGTH_SHORT).show()
                }
                else
                {
                    Toast.makeText(context, "Failed to connect", Toast.LENGTH_SHORT).show()
                }
            }
        }

        btStartButton.setOnClickListener{
            if(hs!!.startBTThread())
            {
                Toast.makeText(context, "Thread Started", Toast.LENGTH_SHORT).show()
            }
            else
            {
                Toast.makeText(context, "Thread failed to start", Toast.LENGTH_SHORT).show()
            }
        }

        btSendButton.setOnClickListener{
            if(!hs!!.sendTest())
            {
                Toast.makeText(context, "Thread not Started", Toast.LENGTH_SHORT).show()
            }
        }


        return rootView
    }

    override fun onDestroy() {
        super.onDestroy()
        hs?.closeConnection()
    }
}
