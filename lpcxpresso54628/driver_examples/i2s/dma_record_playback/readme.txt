Overview
========

The I2S example project uses one I2S interface to continuously record input sound to a buffer
and another I2S interface to playback the buffer to output - digital loopback.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)


Board settings
==============
No special settings are required on LPCXpresso54628 board.

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector.
2.  Connect source of sound(from PC or Smart phone) to Audio Line-In connector.
3.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Headphones will play what is input into the codec Line-In .
Terminal window will show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure WM8904 codec
Configure I2S
Setup digital loopback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
