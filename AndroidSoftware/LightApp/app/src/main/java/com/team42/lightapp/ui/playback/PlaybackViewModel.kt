package com.team42.lightapp.ui.playback

import android.content.Context
import android.widget.Button
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.lifecycle.ViewModel
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.SessionManager
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList

class PlaybackViewModel : ViewModel() {

    fun createButtons(context: Context, layout : TableLayout)
    {
        val sessionList = SessionManager.getSessionList()

        // Create a new button for each saved session
        sessionList.forEachIndexed { index, s ->
            val button = Button(context)
            button.text = s

            val params = TableRow.LayoutParams()
            params.width = 0
            params.height = TableRow.LayoutParams.MATCH_PARENT
            params.weight = 1f
            params.setMargins(8, 0, 8, 0)

            button.layoutParams = params

            // Send this session on click
            button.setOnClickListener {
                val session = SessionManager.getSession(s)
                try {
                    HardwareSystem.uC_SendSession(session)
                }
                catch (e : NullPointerException)
                {
                    Toast.makeText(context, "Device not initialized", Toast.LENGTH_SHORT).show()
                }

            }

            // Add button to table layout
            val row : Int = index / 3

            // If a row exists, get the row
            // If a row does not exists, create one and make the layout the parent
            val rowView = layout.getChildAt(row) as? TableRow ?:
                TableRow(context).also { layout.addView(it) }

            rowView.addView(button)
        }

        // No saved Sessions
        if(sessionList.isEmpty())
        {
            val textbox = TextView(context)
            val string = "No Saved Sessions"
            textbox.text = string

            val rowView = TableRow(context)
            layout.addView(rowView)

            rowView.addView(textbox)
        }


    }

}
