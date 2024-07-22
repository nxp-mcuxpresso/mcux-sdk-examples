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
- Xtensa C Compiler  14.01

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- EVK-MIMXRT595 board sch rev C and higher
- Personal Computer
- Headphones with 3.5 mm stereo jack

Board settings
==============
Note: The I3C Pin configuration in pin_mux.c is verified for default 1.8V, for 3.3V,
need to manually configure slew rate to slow mode for I3C-SCL/SDA.

To enable the example audio using WM8904 codec, connect jumpers as follows:
The 1-2 connected for JP7, JP8, JP27, JP28, JP29.

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect a micro USB cable between the PC host and the debug USB port (J40) on the board
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

### Notes
- DSP image can only be debugged using J-Link debugger. See
'Getting Started with Xplorer for EVK-MIMXRT595.pdf' for more information.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed:
```
    Configure WM8904 codec

    Configure I2S
```

This example transfers data from DMIC to Codec. Connect headphone/earphone on "line-out" of
the board. Speak into the DMIC or play audio near the DMIC (U121), and you will hear sound
on the left channel of headphone/earphone.

