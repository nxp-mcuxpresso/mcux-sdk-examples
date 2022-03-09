Overview
========
The GPIO Example project is a demonstration program that uses the KSDK software to manipulate the general-purpose
outputs.
The example is supported by the set, clear registers for each GPIO pin output register. 

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
When the example runs successfully, the following message is displayed in the terminal:
If you turn on the SW7 , and "SW7 is turned on" is shown on the terminal window

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 GPIO Driver example.

 SW7 is turned on.
 ......
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

