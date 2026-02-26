package com.team42.lightapp

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.drawable.ShapeDrawable
import android.graphics.drawable.shapes.OvalShape
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import androidx.constraintlayout.widget.ConstraintSet

class CanvasView(context: Context) : View(context) {
    val mainPaint : Paint = Paint()

    init {
        mainPaint.setColor(Color.BLUE)
    }

    fun updateLights() {

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

        val CENTER_W : Float = width / 2.0f
        val CENTER_H : Float = height / 2.0f
        val SCALE : Float = 3.0f;

        //canvas.drawPaint(mainPaint)

        fun drawLight(x : Int, y : Int) {
            val newX : Float = x * SCALE + CENTER_W
            val newY : Float = y * SCALE + CENTER_H
            val radius : Float = 20.0f

            canvas.drawCircle(newX, newY, radius, mainPaint)
        }

        drawLight(14, 14)
        drawLight(14, -14)
        drawLight(-14, -14)
        drawLight(-14, 14)

        drawLight(50, 0)
        drawLight(0, 50)
        drawLight(-50, 0)
        drawLight(0, -50)

        drawLight(-63, 63)
        drawLight(63, -63)
        drawLight(-63, -63)
        drawLight(63, 63)

        drawLight(89, 0)
        drawLight(0, 89)
        drawLight(-89, 0)
        drawLight(0, -89)
    }
}