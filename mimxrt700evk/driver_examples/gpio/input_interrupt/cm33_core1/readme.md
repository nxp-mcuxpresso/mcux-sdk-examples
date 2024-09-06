Overview
========
The GPIO input interrupt example is a demonstration program that uses the KSDK software to manipulate the general-purpose
GPIO input interrupt.
The example is supported to set with selection of polarity and edge vs level triggering and all GPIO pins can optionally contribute to one of two GPIO interrupts.

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
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board.
2.  Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
If you press the SWx, the LED will be toggled, and "SWx is pressed" is shown on the terminal window

For CPU0:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 GPIO Driver example

 Press SW5 to turn on/off a LED

 SW5 is pressed
 ......
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For CPU1:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 GPIO Driver example

 Press SW6 to turn on/off a LED

 SW6 is pressed
 ......
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
