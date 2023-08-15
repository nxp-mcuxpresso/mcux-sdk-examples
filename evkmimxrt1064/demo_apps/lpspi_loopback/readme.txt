Overview
========
The lpspi_loopback demo shows how the lpspi do a loopback transfer, LPSPI
master will transmit data to itself, so please connect the SOUT pin to SIN 
pin directly.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1064 board
- Personal Computer

Board settings
==============
Remove the resistor R334 and weld 0Ω resistor to R278,R280.

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       MASTER           connect to           MASTER
Pin Name   Board Location     Pin Name    Board Location
SOUT       J24-4              SIN         J24-5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
***LPSPI Loopback Demo***

LPSPI loopback test pass!!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
