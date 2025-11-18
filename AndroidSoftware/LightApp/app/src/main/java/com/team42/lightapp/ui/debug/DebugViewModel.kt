package com.team42.lightapp.ui.debug

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import androidx.annotation.RequiresPermission
import androidx.core.app.ActivityCompat.startActivityForResult
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.getSystemService
import androidx.core.content.ContextCompat.registerReceiver

import androidx.lifecycle.ViewModel
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.LightSession
import com.team42.lightapp.LightSource
import com.team42.lightapp.SessionBlock
import com.team42.lightapp.SessionManager
import com.team42.lightapp.deleteSession
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList
import com.team42.lightapp.saveSession




class DebugViewModel : ViewModel()
{
    // TODO: Implement the ViewModel

    fun saveSessionTest(hs : HardwareSystem)
    {
        // Delete all saved
        var savedList = SessionManager.getSessionList()

        for (s in savedList)
        {
            SessionManager.deleteSession(s)
        }

        // Should be empty
        savedList = SessionManager.getSessionList()

        hs.sectionCount = 4

        val session = LightSession("Test0")
        val lights = listOf<LightSource>(
            LightSource(0.0,    0.0),
            LightSource(100.0,  1.0),
            LightSource(85.3,   5.0),
            LightSource(0.0,    4.0))

        val lights2 = listOf<LightSource>(
            LightSource(50.0,   1.0),
            LightSource(50.0,   2.0),
            LightSource(50.0,   3.0),
            LightSource(50.0,   4.0))

        val lightsEnd = listOf<LightSource>(
            LightSource(0.0,   0.0),
            LightSource(0.0,   0.0),
            LightSource(0.0,   0.0),
            LightSource(0.0,   0.0))

        session.blocks.add(SessionBlock(lights, 0.0))
        session.blocks.add(SessionBlock(lights2, 5.0))
        session.blocks.add(SessionBlock(lightsEnd, 10.0))

        // Save Test session
        SessionManager.saveSession(hs, session)

        // Save test 2 (copy blocks from test0)
        val session2 = LightSession("Test1", session.blocks)
        SessionManager.saveSession(hs, session2)

        // Check saved, read values in debugger
        savedList = SessionManager.getSessionList()
        var foundSession : LightSession
        for (s in savedList)
        {
            foundSession = SessionManager.getSession(s)
        }
    }

}

