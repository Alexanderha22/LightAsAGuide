package com.team42.lightapp.ui.playback

import android.os.Bundle
import android.text.Layout
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TableLayout
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.team42.lightapp.databinding.FragmentPlaybackBinding
import com.team42.lightapp.R

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
        val layout = root.findViewById<TableLayout>(R.id.tableLayout)

        playbackViewModel.createButtons(requireContext(), layout)
        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}