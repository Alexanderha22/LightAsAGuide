package com.team42.lightapp.ui.playback

import android.content.Context
import android.os.Bundle
import android.text.Layout
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.fragment.app.Fragment
import androidx.fragment.app.commit
import androidx.fragment.app.replace
import androidx.lifecycle.ViewModelProvider
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.databinding.FragmentPlaybackBinding
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager
import com.team42.lightapp.databinding.FragmentDashboardBinding
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList
import com.team42.lightapp.ui.dashboard.DashboardViewModel

class PlaybackFragment : Fragment() {
    private var _binding: FragmentPlaybackBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val playbackViewModel =
            ViewModelProvider(this).get(PlaybackViewModel::class.java)

        _binding = FragmentPlaybackBinding.inflate(inflater, container, false)
        val root: View = binding.root

        val textView: TextView = binding.textPlayback
        playbackViewModel.text.observe(viewLifecycleOwner) {
            textView.text = it
        }
        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}