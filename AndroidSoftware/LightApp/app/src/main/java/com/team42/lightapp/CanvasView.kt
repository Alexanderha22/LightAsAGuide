package com.team42.lightapp

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.Log
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
    var isActive : Boolean = false
}

class CanvasView(context: Context) : View(context) {
    private val mainPaint : Paint = Paint()
    private val selectedPaint : Paint = Paint()

    val lightGroups : MutableList<CanvasLightGroup> = mutableListOf()

    init {
        mainPaint.setColor(Color.BLACK)
        selectedPaint.setColor(Color.MAGENTA)
    }

    fun updateLights() {
        lightGroups.clear()

        //TODO: Determine scale dynamically
        // Find canvas properties
        val CENTER_W : Float = 700.0f / 2.0f
        val CENTER_H : Float = 700.0f / 2.0f
        val SCALE : Float = 3.0f

        for(light in HardwareSystem.ledList) {
            // Assign lights
            val cl : CanvasLight = CanvasLight()
            cl.x = light.x.toFloat() * SCALE + CENTER_W
            cl.y = light.y.toFloat() * SCALE + CENTER_H
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
                    val temp : CanvasLightGroup = CanvasLightGroup()
                    temp.id = lightGroups.size
                    temp.name = "NULL"

                    lightGroups.add(temp)
                }

                // Add this group in there
                lightGroups.add(group)
            }

            // Add the light to the group light list
            group.lightList.add(cl)
        }

        //Redraw canvas
        invalidate()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x : Float = event.x
        val y : Float = event.y

        if(event.action == MotionEvent.ACTION_DOWN) {
            // Check for intersection with touch and light
            fun pointCircleIntersect(px : Float, py : Float,
                                     cx : Float, cy : Float, cr : Float) : Boolean {
                return ((px - cx) * (px - cx) + (py - cy) * (py - cy)) < (cr * cr)
            }

            // Look through all lights, toggle if touched
            // +10 to radius to make it easier to press
            for(group in lightGroups)
                for(light in group.lightList)
                    if(pointCircleIntersect(x, y,
                            light.x, light.y, light.r + 10.0f))
                        group.isActive = !group.isActive

            invalidate()
        }
        return true
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        // Function to draw lights
        fun drawLight(light : CanvasLight, isSelected : Boolean = false) {
            val paint = if (isSelected) selectedPaint else mainPaint
            canvas.drawCircle(light.x, light.y, light.r, paint)
        }

        //Draw all the lights on the screen
        for(group in lightGroups) {
            for(light in group.lightList)
                drawLight(light, group.isActive)
        }
    }
}