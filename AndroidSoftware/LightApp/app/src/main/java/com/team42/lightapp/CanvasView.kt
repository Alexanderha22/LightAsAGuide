package com.team42.lightapp

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.view.MotionEvent
import android.view.View
import com.team42.lightapp.HardwareSystem

class CanvasLight {
    var x : Float = 0.0f
    var y : Float = 0.0f
    var r : Float = 0.0f
}

class CanvasLightGroup {
    var id : Int = 0
    var name : String = "No Name"
    var lightList : MutableList<CanvasLight> = mutableListOf()
}

class CanvasView(context: Context) : View(context) {
    val mainPaint : Paint = Paint()

    val lightGroups : MutableList<CanvasLightGroup> = mutableListOf()
    val lightList : MutableList<CanvasLight> = mutableListOf()

    init {
        mainPaint.setColor(Color.BLUE)
    }

    fun updateLights() {
        lightGroups.clear()
        lightList.clear()

        for(light in HardwareSystem.ledList) {
            // Assign lights
            var cl : CanvasLight = CanvasLight()
            cl.x = light.x.toFloat()
            cl.y = light.y.toFloat()
            cl.r = 20.0f;

            // Check if group has been made yet
            var group : CanvasLightGroup? = null;
            if(lightGroups.size > light.section)
                group = lightGroups[light.section]
            else {
                // Create a new group
                group = CanvasLightGroup()
                group.id = light.section
                group.name = "Group ${group.id}"

                // Add groups until the index is found
                while(lightGroups.size < group.id)
                {
                    var temp : CanvasLightGroup = CanvasLightGroup()
                    temp.id = lightGroups.size
                    temp.name = "NULL"

                    lightGroups.add(temp)
                }

                // Add this group in there
                lightGroups.add(group)
            }

            // Add the light to the group list and the main list
            group.lightList.add(cl)
            lightList.add(cl)
        }

        //Redraw canvas
        invalidate()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x : Float = event.x
        val y : Float = event.y

        if(event.action == MotionEvent.ACTION_DOWN) {
            //TODO: Look through all lights and see what intersects
            //TODO: (De)activate lights that are touched

            //TODO: Remove when above is implemented
            mainPaint.color = if (mainPaint.color == Color.BLUE) Color.GREEN else Color.BLUE
            invalidate()
        }
        return true
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        //TODO: Determine scale dynamically
        // Find canvas properties
        val CENTER_W : Float = width / 2.0f
        val CENTER_H : Float = height / 2.0f
        val SCALE : Float = 3.0f;

        // Function to draw lights
        fun drawLight(light : CanvasLight) {
            val newX : Float = light.x * SCALE + CENTER_W
            val newY : Float = light.y * SCALE + CENTER_H
            val radius : Float = light.r

            canvas.drawCircle(newX, newY, radius, mainPaint)
        }

        //Draw all the lights on the screen
        for(light in lightList)
            drawLight(light)
    }
}