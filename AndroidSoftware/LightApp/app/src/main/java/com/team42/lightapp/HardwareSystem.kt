package com.team42.lightapp

import android.hardware.lights.Light
import android.se.omapi.Session


class HardwareSystem
{
    fun sendSession(session : List<SessionBlock>) {}
    fun stopSession(){}
    fun setLight(index : Int, source : LightSource){}

    fun ringFromLight(lightIndex : Int) {}
}

class SessionBlock
(
    val lights  : Array<LightSource>    = Array(4) {LightSource()},
    val timeStamp  : Int                   = 0,
){
    fun setLight(index : Int, source : LightSource){}

}

class LightSource
(
    val brightness  : Int = 0,
    val frequency   : Int = 0
){}


