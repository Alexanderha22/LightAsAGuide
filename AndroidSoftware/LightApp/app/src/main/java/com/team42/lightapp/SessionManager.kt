package com.team42.lightapp

import android.hardware.lights.Light
import android.se.omapi.Session

object SessionManager
{
    // Returns a list of the saved sessions' names
    fun getSessionList() : list<String> {return emptyList<String>()};

    // Returns the LightSession corresponding to the name
    fun getSession() : LightSession {return emptyList<SessionBlock>()};

    // Saves the session with the name
    fun SaveSession(sessionName : String) {};
}