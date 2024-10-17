Overview
========
The acomp basic driver example demostrates the basic usage of the ACOMP module. This example compares the user input
analog signal with interanl reference voltage(VDDIO_3 * 0.5) and will toggle the LED when the result changes. The purpose
of this demo is to show how to use the ACOMP driver in SDK software. In this driver example the 'outPinMode' is set
as 'kACOMP_PinOutSynInverted', so the output signal from gpio is the inversion of the comparator actual output signal.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings.

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ACOMP Basic Example!

Please press any key to get the ACOMP execute result.


The positive input voltage is greater than negative input voltage!


The positive input voltage is less than negative input voltage!


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The analog signal input voltage range is 0-3.3v. Connect the analog signal to J4-2(GPIO_42),
if input analog signal's voltage greater than 1.65V then serial print "The positive input voltage is greater than negative input voltage!"
otherwise, the serial port print "The positive input voltage is less than negative input voltage!".
