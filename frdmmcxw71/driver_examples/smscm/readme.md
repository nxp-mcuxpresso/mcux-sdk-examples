Overview
========
The SMSCM demo shows how to use the secure siscellaneous system control module using smscm driver. 
In this demo, the security counter register is set to a value, and it will be plus and minus a fixed number with
security counter plus X register or security counter minus X register.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the FRDM board J10.
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
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SMSCM example start
The value of Security Counter Register (SCTR) is: 0xff
Write the value x to Plus Register: x = 3.
The value in security counter is plused to 0x102 successfully!
Write the value x to Minus Register: x = 2.
The value in security counter is minused to 0x100 successfully!
SMSCM example finished

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
