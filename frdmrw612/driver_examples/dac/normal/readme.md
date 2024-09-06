Overview
========
The DAC normal driver example demostrates the basic useage of DAC module. In this example, users input value from the
terminal, and then the related voltage will be output through DAC output pin.

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
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
No special setting

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
The log below shows the output of the dac normal demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DAC Normal Driver Example!

Output = 0.18V + (1.42V * input_code / 1023)

Please input the value(Ranges from 0 to  1023) to be converted.
1000
Please input the value(Ranges from 0 to  1023) to be converted.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use the oscilloscope or universal meter to probe J4-4(GPIO_43), then the output voltage can be measured.
