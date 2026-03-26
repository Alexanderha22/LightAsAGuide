package com.team42.lightapp

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.hardware.lights.Light
import android.view.View
import android.graphics.Path
import kotlin.math.sin

class WaveVisualizerView(context: Context) : View(context) {
    private val mainPaint : Paint = Paint().apply {
        color = Color.BLUE
        style = Paint.Style.STROKE
        strokeWidth = 5f
        isAntiAlias = true
    }

    private var sources : MutableList<LightSource> = mutableListOf()

    fun updateSection(source : LightSource, index : List<Int>)
    {
        index.forEach{i ->
            while(i >= sources.size)
            {
                sources.add(LightSource())
            }
            sources[i] = source
        }
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        canvas.drawARGB(0xFF, 0xEB, 0xC7, 0xB2)

        // Function to draw lights
        fun drawWave(source : LightSource) {
            val path = Path()

            path.moveTo(0f, height / 2.0f)

            for (x in 0..width.toInt()) {
                val y = height / 2.0f + (height * source.brightness / 200.0f) *
                        sin((x.toFloat() / width) * source.frequency).toFloat()
                path.lineTo(x.toFloat(), y.toFloat())
            }

            canvas.drawPath(path, mainPaint)
        }

        //Draw all the lights on the screen
        sources.forEachIndexed{i, source ->
            mainPaint.setColor(GROUP_COLORS[i])
            drawWave(source)
        }
    }
}