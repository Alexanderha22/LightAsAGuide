package com.team42.lightapp.ui.debug

import android.util.Log
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
        Log.d("SaveTest", "Saved List Length Before: ${savedList.count()}")

        for (s in savedList)
        {
            SessionManager.deleteSession(s)
        }

        // Should be empty
        savedList = SessionManager.getSessionList()
        Log.d("SaveTest", "Saved List Length After: ${savedList.count()}")

        hs.sectionCount = 4

        val session = LightSession("Test0")
        val lights = mutableListOf(
            LightSource(0.0,    0.0),
            LightSource(100.0,  1.0),
            LightSource(85.3,   5.0),
            LightSource(0.0,    4.0))

        val lights2 = mutableListOf(
            LightSource(50.0,   1.0),
            LightSource(50.0,   2.0),
            LightSource(50.0,   3.0),
            LightSource(50.0,   4.0))

        val lightsEnd = mutableListOf(
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
            Log.d("SaveTest", "Found Session Name: $s")
        }
    }

}

