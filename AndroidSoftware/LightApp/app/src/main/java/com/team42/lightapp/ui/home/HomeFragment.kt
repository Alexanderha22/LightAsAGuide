package com.team42.lightapp.ui.home

import android.os.Bundle
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


        var LED0B : Double = 0.0
        var LED0F : Double = 0.0

        fun Send()
        {
            try {
                val ls : LightSource = LightSource(LED0B, LED0F)
                HardwareSystem.uC_SetSection(0, ls)
            }
            catch (err : NullPointerException)
            {
                Toast.makeText(requireContext(), "Device not initialized", Toast.LENGTH_SHORT).show()
            }
        }


        val homeViewModel =
            ViewModelProvider(this).get(HomeViewModel::class.java)

        _binding = FragmentHomeBinding.inflate(inflater, container, false)
        val root: View = binding.root

        //val textView: TextView = binding.textHome
        //homeViewModel.text.observe(viewLifecycleOwner) {
        //    textView.text = it
        //}

        val switch: Switch = binding.homeSwitchStatus
        switch.isChecked

        //BRIGHTNESS
        val brightnessNum: EditText = binding.homeEditBrightness
        val brightnessSeek: SeekBar = binding.homeBrightnessSeek

        //Change number when brightness bar changes
        brightnessSeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, progress: Int, fromUser: Boolean) {
                brightnessNum.setText(progress.toString())
                LED0B = progress.toDouble()
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }
            override fun onStopTrackingTouch(bar: SeekBar?) {
                Send()
            }
        })

        //Change brightness bar when number changes
        brightnessNum.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(editable: Editable?) {;
                val n: Int = try {
                    editable.toString().toInt();
                } catch(e : NumberFormatException) {
                    brightnessSeek.progress
                }

                brightnessSeek.setProgress(n, true)
            }

            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
        })


        //FREQUENCY
        val frequencyNum: EditText = binding.homeEditFrequency
        val frequencySeek: SeekBar = binding.homeFrequencySeek

        //Change number when brightness bar changes
        frequencySeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, progress: Int, fromUser: Boolean) {
                frequencyNum.setText(progress.toString())
                LED0F = progress.toDouble()
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }
            override fun onStopTrackingTouch(bar: SeekBar?) {
                Send()
            }
        })

        //Change brightness bar when number changes
        frequencyNum.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(editable: Editable?) {
                val n: Int = try {
                    editable.toString().toInt()
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