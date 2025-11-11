package com.team42.lightapp.ui.debug


import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager


class DebugFragment : Fragment() {

    companion object {
        fun newInstance() = DebugFragment()
    }

    private val viewModel: DebugViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // TODO: Use the ViewModel
        // Setup sessionmanager
        SessionManager.getFolder(requireContext())
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val rootView = inflater.inflate(R.layout.fragment_debug, container, false)

        // Find the button by its ID
        val saveButton: Button = rootView.findViewById(R.id.savebutton)

        // Set a click listener on the button
        saveButton.setOnClickListener {
            // Handle the button click event here
            viewModel.saveSessionTest()
        }

        return rootView
    }
}