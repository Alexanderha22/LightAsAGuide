package com.team42.lightapp.ui.dashboard

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager
import com.team42.lightapp.databinding.FragmentDashboardBinding
import com.team42.lightapp.databinding.FragmentPlaybackBinding
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList
import com.team42.lightapp.ui.playback.PlaybackViewModel

class DashboardFragment : Fragment() {

    private var _binding: FragmentDashboardBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    fun createButtons(context: Context, layout : TableLayout)
    {
        val dashboardViewModel =
            ViewModelProvider(this).get(DashboardViewModel::class.java)

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
                dashboardViewModel.lightSession.value = session

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

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val dashboardViewModel =
            ViewModelProvider(this).get(DashboardViewModel::class.java)

        _binding = FragmentDashboardBinding.inflate(inflater, container, false)
        val root: View = binding.root
        val layout = root.findViewById<TableLayout>(R.id.tableLayout)

        createButtons(requireContext(), layout)

        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}