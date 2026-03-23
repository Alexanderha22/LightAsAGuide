package com.team42.lightapp.ui.playback

import android.content.Context
import android.widget.Button
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.SessionManager
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList
import com.team42.lightapp.LightSession
import com.team42.lightapp.EMPTY_SESSION

class PlaybackViewModel : ViewModel() {
    private val _text = MutableLiveData<String>().apply {
        value = "This"
    }

    //SessionFragment will reference this as the active light session
    var lightSession = MutableLiveData<LightSession>().apply {
        value = EMPTY_SESSION
    }
}
