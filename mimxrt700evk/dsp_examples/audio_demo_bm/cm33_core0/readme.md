Overview
========
The audio baremetal demo demonstrates audio processing using DSP core.
It uses the DMIC working with I2S. One channel Audio data is converted to samples in the DMIC module.
Then, the data is placed into the memory buffer. Last, it is sent to the I2S
buffer and output to the CODEC, where the audio data will be heard from lineout.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT700-EVK board
- Personal Computer
- Headphones with 3.5 mm stereo jack

Board settings
==============

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J29).
2.  Connect a micro USB cable between the PC host and the debug USB port (J54) on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program for CM33 core to the target board.
5.  Launch the debugger in your IDE to begin running the demo.
6.  If building debug configuration, download the program for DSP core to the target board.
7.  If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to
begin running the demo.

NOTE: DSP image can only be debugged using J-Link debugger.  See
'Getting Started with Xplorer for MIMXRT700-EVK.pdf' for more information.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.

[CM33 Main] Audio demo started. Initialize pins and codec on core 'Cortex-M33'
[CM33 Main] Configure codec
[CM33 Main] Pins and codec initialized.
[DSP Main] DSP starts on core 'rt700_hifi4_RI23_11_nlib'
[DSP Main] PDM->DMA->SAI->CODEC running 

This example transfers data from DMIC to Codec. Connect headphone/earphone on audio out of the board.
Speak into the DMIC or play audio near the DMIC (U66), and you will hear sound on the left channel of
headphone/earphone.
