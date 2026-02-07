package com.team42.lightapp.ui.debug


import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.LightSession
import com.team42.lightapp.LightSource
import com.team42.lightapp.R
import com.team42.lightapp.SessionBlock
import com.team42.lightapp.SessionManager
import com.team42.lightapp.getSession
import com.team42.lightapp.saveSession

class DebugFragment : Fragment() {

    companion object {
        fun newInstance() = DebugFragment()
    }

    private val viewModel: DebugViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // TODO: Use the ViewModel
        // Setup sessionManager
        SessionManager.getFolder(requireContext())

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
        val permissionButton : Button = rootView.findViewById(R.id.getPermission)
        val btSendSessionButton : Button = rootView.findViewById(R.id.btSendSession)

        // Set a click listener on the button
        saveButton.setOnClickListener {
            // Handle the button click event here
            viewModel.saveSessionTest(HardwareSystem)
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
                    requireContext() as Activity,
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
                Toast.makeText(requireContext(), "Permissions already Granted", Toast.LENGTH_SHORT).show()
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
                if(HardwareSystem.connectToPairedDevice(requireContext()))
                {
                    Toast.makeText(requireContext(), "Connected", Toast.LENGTH_SHORT).show()
                }
                else
                {
                    Toast.makeText(requireContext(), "Failed to connect", Toast.LENGTH_SHORT).show()
                }
            }
        }

        btSendSessionButton.setOnClickListener{
            // Create test session
            HardwareSystem.sectionCount = 4

            val session = LightSession("CDRDemo")
            val lights = mutableListOf(
                LightSource(0.0,    0.0),
                LightSource(100.0,  1.0),
                LightSource(85.3,   5.0),
                LightSource(0.0,    4.0)
            )

            val lights2 = mutableListOf(
                LightSource(50.0,   1.0),
                LightSource(50.0,   2.0),
                LightSource(50.0,   3.0),
                LightSource(50.0,   4.0)
            )

            val lightsEnd = mutableListOf(
                LightSource(0.0,   0.0),
                LightSource(0.0,   0.0),
                LightSource(0.0,   0.0),
                LightSource(0.0,   0.0)
            )

            session.blocks.add(SessionBlock(lights, 0.0))
            session.blocks.add(SessionBlock(lights2, 5.0))
            session.blocks.add(SessionBlock(lightsEnd, 10.0))

            try {
                HardwareSystem.uC_SendSession(session)
            }
            catch (err : NullPointerException)
            {
                Toast.makeText(requireContext(), "Device not initialized", Toast.LENGTH_SHORT).show()
            }
        }


        rootView.findViewById<Button>(R.id.btDisconnect).setOnClickListener{
            if(HardwareSystem.closeConnection())
            {
                Toast.makeText(requireContext(), "Disconnected", Toast.LENGTH_SHORT).show()
            }
        }
        fun printParseResult()
        {
            Log.d("Parse", "SectionCount: ${HardwareSystem.sectionCount}")
            for(info in HardwareSystem.ledList)
            {
                Log.d("Parse", "X:${info.x} Y:${info.y} S:${info.section}")
            }

            HardwareSystem.externalModuleMap.forEach {
                    eID, module -> Log.d("Parse", "EID:$eID Name:${module.name} Description:${module.description}")
            }
        }
        rootView.findViewById<Button>(R.id.btParseTest).setOnClickListener{
            var incomingMessage = "SetInfo,3,3," +
                    "0,0,0," +
                    "0,1,1," +
                    "1,1,2"

            HardwareSystem.parseTest(requireContext(), incomingMessage)
            printParseResult()

            incomingMessage = "SetInfo,10,2,"
            for(i in 0 until 10)
            {
                incomingMessage += "$i,$i,${i%2},"
            }
            incomingMessage += "23,Input,E Module,This is an external device"
            HardwareSystem.parseTest(requireContext(), incomingMessage)
            printParseResult()

        }

        rootView.findViewById<Button>(R.id.btSetSingle).setOnClickListener{
            try {
                val ls : LightSource = LightSource(50.0, 10.0)
                HardwareSystem.uC_SetSection(3,ls)
            }
            catch (err : NullPointerException)
            {
                Toast.makeText(requireContext(), "Device not initialized", Toast.LENGTH_SHORT).show()
            }

        }

        return rootView
    }

    override fun onDestroy() {
        super.onDestroy()
    }
}
