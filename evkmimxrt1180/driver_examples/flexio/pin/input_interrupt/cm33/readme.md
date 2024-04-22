Overview
========
The FLEXIO PIN Example project is a demonstration program that uses the FLEXIO software to manipulate 
the flexio-pin as input function.
The example uses FLEXIO-PIN input to capture the edge of other gpio pin.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
To make this example work, connections needed to be as follows:

    FLEXIO_PIN        connected to  RGPIO
PIN0     J69-3           -->        J69-4

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCULink USB port on the target board. 
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
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 FLEXIO PIN Driver example

 The output pin is taking turns to occurr rising and falling edge.

Toggle output pin , FLEXIO PIN interrupt rising interrupt occurred 

Toggle output pin , FLEXIO PIN interrupt didn't occurr

Toggle output pin , FLEXIO PIN interrupt rising interrupt occurred 

Toggle output pin , FLEXIO PIN interrupt didn't occurr
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
