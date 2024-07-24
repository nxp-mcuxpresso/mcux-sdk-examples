Overview
========
The voice_spot_demo application demonstrates Voice Spot initialization and processing.
It gets the audio data from the on-board microphone, looks for a wake word (Hey NXP) and outputs
the audio data to the speaker.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer
- Microphone

Board settings
==============
Connect microphone module to EVK board:
3V->J20-8
GND->J20-7
CLK->J21-4
DATA->J21-3

#### Please note this application can't support running with Linux BSP! ####

#### Note also that the Debug target in Xtensa IDE does not build because of memory being too small.
#### Please use only Release target.

Prepare the Demo
================
1.  Connect 5V power supply and JLink Plus to the board, switch SW10 to power on the board
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs successfully, you can see init logs on terminal like:

	Voice spot demo started. Initialize pins and codec on core 'Cortex-M33'
	Pins and codec initialized.
	VoiceSpot example started on core 'fusion_nxp02_dsp_prod'
	rdspVoiceSpot_CreateControl: voicespot_status = 0
	rdspVoiceSpot_CreateInstance: voicespot_status = 0
	rdspVoiceSpot_OpenInstance: voicespot_status = 0
	rdspVoiceSpot_EnableAdaptiveThreshold: voicespot_status = 0
	VoiceSpot library version: 0.23.4.1668162583
	VoiceSpot model version: 0.13.1
	VoiceSpot model string: HeyNXP_en-US_1
	Voice spot init done.

Then when user says "hey NXP", a line similar to this will be printed (all values may differ):

	Trigger event found: Event = 0, Frame = 344, class_string = HeyNXP, Score = 112, trigger_sample = 69000, estimation_sample = 69000, start_offset_sam

