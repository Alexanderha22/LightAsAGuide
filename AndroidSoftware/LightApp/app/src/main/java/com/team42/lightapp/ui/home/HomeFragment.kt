package com.team42.lightapp.ui.home

import android.hardware.lights.Light
import android.os.Bundle
import android.os.SystemClock
import android.text.Editable
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.SeekBar
import android.widget.Switch
import androidx.core.widget.addTextChangedListener
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.databinding.FragmentHomeBinding

import com.team42.lightapp.LightSource
import android.widget.Toast

class HomeFragment : Fragment() {

    private var _binding: FragmentHomeBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        val homeViewModel =
            ViewModelProvider(this).get(HomeViewModel::class.java)

        _binding = FragmentHomeBinding.inflate(inflater, container, false)
        val root: View = binding.root

        var currentIndex = 0

        // ON OFF
        val switch: Switch = binding.homeSwitchStatus

        // BRIGHTNESS
        val brightnessNum: EditText = binding.homeEditBrightness
        val brightnessSeek: SeekBar = binding.homeBrightnessSeek

        // FREQUENCY
        val frequencyNum: EditText = binding.homeEditFrequency
        val frequencySeek: SeekBar = binding.homeFrequencySeek

        // Only send every 1000? ms
        val minDeltaTime = 10
        var lastSendTime : Long = 0

        // Send data to hardware
        fun Send()
        {
            val now = SystemClock.elapsedRealtime()
            if(now < lastSendTime + minDeltaTime)
            {
                return
            }
            lastSendTime = now

            try {
                if(switch.isChecked)
                {
                    val ls : LightSource = LightSource(
                        brightnessSeek.progress.toDouble() / 10.0,
                        frequencySeek.progress.toDouble() / 100.0)

                    HardwareSystem.uC_SetSection(currentIndex, ls)
                }
                else {
                    val ls : LightSource = LightSource(0.0,0.0)
                    HardwareSystem.uC_SetSection(currentIndex, ls)
                }
            }
            catch (e: Exception) {
                // If there is an error, toast it
                when (e) {
                    is NullPointerException -> Toast.makeText(requireContext(), "Device not initialized", Toast.LENGTH_SHORT).show()
                    else -> Toast.makeText(requireContext(), e.message, Toast.LENGTH_SHORT).show()
                }
            }
        }

        switch.setOnClickListener{
            Send()
        }


        // Update Brightness

        // Change number when brightness bar changes
        brightnessSeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, progress: Int, fromUser: Boolean) {
                brightnessNum.setText((progress / 10.0).toString())
                Send()
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }

            // Send when done tracking
            override fun onStopTrackingTouch(bar: SeekBar?) { return }
        })

        // Change brightness bar when number changes
        brightnessNum.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(editable: Editable?) {;
                val n: Int = try {
                    (editable.toString().toDouble() * 10.0).toInt()
                } catch(e : NumberFormatException) {
                    brightnessSeek.progress
                }

                brightnessSeek.setProgress(n, true)
            }

            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
        })

        // Update Frequency

        // Change number when brightness bar changes
        frequencySeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, progress: Int, fromUser: Boolean) {
                frequencyNum.setText((progress / 100.0).toString())
                Send()
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }

            // Send when done tracking
            override fun onStopTrackingTouch(bar: SeekBar?) { return }
        })

        // Change frequency  bar when number changes
        frequencyNum.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(editable: Editable?) {
                val n: Int = try {
                    (editable.toString().toDouble() * 100).toInt()
                } catch(e : NumberFormatException) {
                    frequencySeek.progress
                }

                frequencySeek.setProgress(n, true)
            }

            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
        })

        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}