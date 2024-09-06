Overview
========
The sdadc sai interrupt example shows how to use sdadc driver with interrupt.
In this example, sdadc gathers analog data from the microphone, and uses the
sai to send the digital data to the codec. The user can hear the sound from
the microphone and see the log on the terminal.

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
- Headphone

Board settings
==============
Connect the headphone to the J29 port.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board. 
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
When the demo runs successfully, you can hear the sound from the microphone(U62, U65) and the log can be seen on the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 SDADC SAI Interrupt Example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
