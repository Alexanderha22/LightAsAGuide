package com.team42.lightapp.ui.playback

import android.content.Context
import android.os.Bundle
import android.os.SystemClock
import android.text.Editable
import android.text.Layout
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.SeekBar
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import android.widget.Toast
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.databinding.Observable
import androidx.databinding.ObservableInt
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import androidx.fragment.app.commit
import androidx.fragment.app.replace
import androidx.lifecycle.ViewModelProvider
import com.team42.lightapp.DashboardCanvasView
import com.team42.lightapp.EMPTY_SESSION
import com.team42.lightapp.EditorCanvasView
import com.team42.lightapp.HardwareSystem
import com.team42.lightapp.LightSource
import com.team42.lightapp.databinding.FragmentPlaybackBinding
import com.team42.lightapp.R
import com.team42.lightapp.SessionManager
import com.team42.lightapp.WaveVisualizerView
import com.team42.lightapp.databinding.FragmentDashboardBinding
import com.team42.lightapp.databinding.FragmentHomeBinding
import com.team42.lightapp.getSession
import com.team42.lightapp.getSessionList
import com.team42.lightapp.ui.dashboard.DashboardViewModel
import com.team42.lightapp.ui.home.HomeViewModel
import kotlin.math.max
import kotlin.math.min

class PlaybackFragment : Fragment() {
    private var _binding: FragmentPlaybackBinding? = null

    private var activeBlockIndex: Int = 0
    private var activeSection: Int = -1

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

        val dashboardViewModel : DashboardViewModel by activityViewModels()
        var lightSession = dashboardViewModel.lightSession.value!!

        _binding = FragmentPlaybackBinding.inflate(inflater, container, false)
        val root: View = binding.root

        /*val textView: TextView = binding.textPlayback
        playbackViewModel.text.observe(viewLifecycleOwner) {
            textView.text = it
        }*/

        // NAME
        val sessionNameInput : EditText = binding.playbackSessionName

        // OFF
        val startButton : Button = binding.playbackStartButton

        // LED MAP
        val mapLayout : FrameLayout = binding.playbackLightDrawable
        val mapCanvasView = EditorCanvasView(requireContext())
        mapLayout.addView(mapCanvasView)

        // BRIGHTNESS
        val brightnessNum: EditText = binding.playbackEditBrightness
        val brightnessSeek: SeekBar = binding.playbackBrightnessSeek

        // FREQUENCY
        val frequencyNum: EditText = binding.playbackEditFrequency
        val frequencySeek: SeekBar = binding.playbackFrequencySeek

        //TIME
        val blockTime: EditText = binding.playbackEditTime

        // BLOCK HANDLER
        val prevBlockButton: Button = binding.playbackPrevBlock
        val nextBlockButton: Button = binding.playbackNextBlock
        val blockNum: TextView = binding.playbackBlockNum
        val blockCount: TextView = binding.playbackBlockCount

        // Only send every 1000? ms
        val minDeltaTime = 100
        var lastSendTime : Long = 0

        // Send data to hardware
        fun Send(ignoreDelay : Boolean = false)
        {
            val now = SystemClock.elapsedRealtime()
            if(!ignoreDelay && (now < lastSendTime + minDeltaTime))
            {
                return
            }
            lastSendTime = now

            // Get a list of active groups
            val activeGroups : MutableList<Int> = mutableListOf()
            mapCanvasView.lightGroups.forEach { group ->
                if(group.isActive)
                    activeGroups.add(group.id)
            }

            if(activeGroups.isEmpty())
                return

            try {
                val ls : LightSource = LightSource(
                    brightnessSeek.progress.toDouble() / 10.0,
                    frequencySeek.progress.toDouble() / 100.0)

                // Send one set section command for all active groups
                HardwareSystem.uC_SetSection(ls, activeGroups.toList())

            }
            catch (e: Exception) {
                // If there is an error, toast it
                when (e) {
                    is NullPointerException -> Toast.makeText(requireContext(), "Device not initialized", Toast.LENGTH_SHORT).show()
                    else -> Toast.makeText(requireContext(), e.message, Toast.LENGTH_SHORT).show()
                }
            }
        }

        //Reset the section
        activeSection = -1

        //Get the section when it's changed
        mapCanvasView.activeGroupId.addOnPropertyChangedCallback(object :
            Observable.OnPropertyChangedCallback() {
            override fun onPropertyChanged(sender: Observable?, propertyId: Int) {
                activeSection = (sender as ObservableInt).get()
                var brightness: Double = 0.0
                var frequency: Double = 0.0
                var time: Double = 0.0

                //Set the sliders to what the sections are
                if(activeSection >= 0) {
                    brightness =
                        lightSession.blocks[activeBlockIndex].lights[activeSection].brightness
                    frequency =
                        lightSession.blocks[activeBlockIndex].lights[activeSection].frequency

                    //Get the time difference
                    if(activeBlockIndex < lightSession.blocks.size - 1)
                        time = (lightSession.blocks[activeBlockIndex + 1].timeStamp -
                                lightSession.blocks[activeBlockIndex].timeStamp)
                }

                brightnessNum.setText(brightness.toString())
                frequencyNum.setText(frequency.toString())
                blockTime.setText(time.toString())
            }
        })

        //Set the name from the current light session
        sessionNameInput.setText(lightSession.name)

        //Afterwards, set the name if the user changes it
        sessionNameInput.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(editable: Editable?) {
                lightSession.name = editable.toString()
            }

            // Not needed
            override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
            override fun onTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) { return }
        })

        //Send session when pressed
        startButton.setOnClickListener{
            try {
                HardwareSystem.uC_SendSession(lightSession)
            }
            catch (e : NullPointerException)
            {
                Toast.makeText(context, "Device not initialized", Toast.LENGTH_SHORT).show()
            }
        }


        // Update Brightness

        // Change number when brightness bar changes
        brightnessSeek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, progress: Int, fromUser: Boolean) {
                brightnessNum.setText((progress / 10.0).toString())
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }

            // Send when done tracking
            override fun onStopTrackingTouch(bar: SeekBar?) { Send(true) }
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
            }

            //Unused
            override fun onStartTrackingTouch(bar: SeekBar?) { return }

            // Send when done tracking
            override fun onStopTrackingTouch(bar: SeekBar?) { Send(true) }
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

        // Check if the current block index is out of bounds
        activeBlockIndex = min(0, max(lightSession.blocks.size - 2, activeBlockIndex))

        // Set the block time
        blockTime.setText((lightSession.blocks[activeBlockIndex + 1].timeStamp -
                lightSession.blocks[activeBlockIndex].timeStamp).toString())

        // Set the index
        blockNum.text = (activeBlockIndex + 1).toString()

        // Set the block count (do not count the ending block which is always all off)
        blockCount.text = (lightSession.blocks.size - 1).toString()

        // Decrease block on click
        prevBlockButton.setOnClickListener{
            if(activeBlockIndex > 0) {
                --activeBlockIndex
                blockNum.text = (activeBlockIndex + 1).toString()
                mapCanvasView.clearActiveGroups()

                brightnessNum.setText("0.0")
                frequencyNum.setText("0.0")
                blockTime.setText((lightSession.blocks[activeBlockIndex + 1].timeStamp -
                        lightSession.blocks[activeBlockIndex].timeStamp).toString())
            }
        }

        // Increase block on click
        nextBlockButton.setOnClickListener{
            if(activeBlockIndex < lightSession.blocks.size - 2) {
                ++activeBlockIndex
                blockNum.text = (activeBlockIndex + 1).toString()
                mapCanvasView.clearActiveGroups()

                brightnessNum.setText("0.0")
                frequencyNum.setText("0.0")
                blockTime.setText((lightSession.blocks[activeBlockIndex + 1].timeStamp -
                        lightSession.blocks[activeBlockIndex].timeStamp).toString())
            }
        }


        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}