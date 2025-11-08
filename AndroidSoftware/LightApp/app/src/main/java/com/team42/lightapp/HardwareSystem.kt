package com.team42.lightapp

import android.hardware.lights.Light
import android.se.omapi.Session

class LightSession(
    val name : String,
    val blocks : MutableList<SessionBlock> = mutableListOf<SessionBlock>()
)
{}

class HardwareSystem()
{
    // Hardware information
    var sectionCount = 0
    private var ledList : List<LEDInfo> = emptyList()
    fun getLED(ledID : Int) : LEDInfo {return LEDInfo()}

    // Call getInfo, set hardware info
    init{}

    // Functions to communicate with the microcontroller
    fun uC_SendSession(session : LightSession) {}
    fun uC_StopSession(){}
    fun uC_SetSection(index : Int, source : LightSource){}
    fun uC_GetInfo(){}
    fun uC_GetUpdates(index : Int, receiveUpdates : Boolean, frequency: Int){}
}

class SessionBlock
(
    val lights  : List<LightSource> = emptyList<LightSource>(),
    val timeStamp  : Double = 0.0,
){
    fun setSection(index : Int, source : LightSource){}

}

class LightSource
(
    val brightness  : Double = 0.0,
    val frequency   : Double = 0.0
){}

class LEDInfo(
    val id : Int = 0,
    val x : Int = 0,
    val y : Int = 0,
    val section : Int = 0
) {}
