package com.team42.lightapp.ui.dashboard

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.team42.lightapp.EMPTY_SESSION
import com.team42.lightapp.LightSession

class DashboardViewModel : ViewModel() {

    private val _text = MutableLiveData<String>().apply {
        value = "This"
    }

    //SessionFragment will reference this as the active light session
    var lightSession = MutableLiveData<LightSession>(EMPTY_SESSION)
}