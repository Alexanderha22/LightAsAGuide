package com.team42.lightapp

import android.hardware.lights.Light
import android.se.omapi.Session

typealias LightSession = List<SessionBlock>;

class HardwareSystem()
{
    // Hardware information
    var sectionCount = 0;
    var ledList : List<LEDInfo> = emptyList();

    // Call getInfo, set hardware info
    init{};

    // Functions to communicate with the microcontroller
    fun sendSession(session : LightSession) {}
    fun stopSession(){}
    fun setSection(index : Int, source : LightSource){}

    fun getInfo(){};
}

class SessionBlock
(
    val lights  : List<LightSource> = emptyList<LightSource>(),
    val timeStamp  : Int = 0,
){
    fun setSection(index : Int, source : LightSource){}

}

class LightSource
(
    val brightness  : Int = 0,
    val frequency   : Int = 0
){}

class LEDInfo(
    val id : Int = 0,
    val x : Int = 0,
    val y : Int = 0,
    val section : Int = 0
) {}
