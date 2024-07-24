Overview
========
The dsp_pdm_sai demo demonstrates audio processing using DSP core.
It uses the PDM and SAI driver to capture and playback the audio.
One channel audio data is converted to samples in the PDM module.
Then, the data is placed into the memory buffer by DMA. Last, it is sent to the I2S
buffer and output to the CODEC, where the audio data will be heard from lineout.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- Xtensa C Compiler  14.01

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
When the demo runs successfully, you can hear the sound gathered from microphone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
Audio demo started. Initialize pins and codec on core 'Cortex-M33'
Pins and codec initialized.
DSP starts on core 'fusion_nxp02_dsp_prod'
MIC->DMA->I2S->CODEC running 
~~~~~~~~~~~~~~~~~~~
