Overview
========

The tstmr example shows the usage of TSTMR driver in application. The TSTMR module is a free running incrementing counter that starts running after system reset de-assertion and can be read at any time by the software for determining the software ticks.The TSTMR runs off the 1 MHz clock and resets on every system reset.

In this example, it would output a time stamp information when the application is ready. And then, delay for 1 second with TSTMR_DelayUs() function. Before and after the delay, it would output the two time stamps information again.


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Timestamp1 = 0627

 Test the delay function, delay for 1 second

 Start time = 01e69

 End time = 0f689e
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
