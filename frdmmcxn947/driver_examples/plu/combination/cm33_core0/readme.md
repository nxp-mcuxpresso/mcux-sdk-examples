Overview
========
The PLU combination project is a simple demonstration program of the SDK PLU driver. In this example, a number of 
GPIO pins are connected to PLU inputs and the LED is used to monitor the PLU outputs to demonstrate the 
configuration and use of the Programmable Logic Unit (PLU) to construct a combinational logic network.

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
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
PLU input pins connection:
- J8-23 (GPIO4_18) to J1-4 (PLU_IN2).
- J8-24 (GPIO4_19) to J1-2 (PLU_IN3).
- J8-25 (GPIO4_20) to J8-6 (PLU_IN5).

PLU output pins connection:
- J8-17 (PLU_OUT0) to J2-4 (LED_RED).
- J8-18 (PLU_OUT1) to J2-6 (LED_GREEN).
- J8-19 (PLU_OUT2) to J1-14 (LED_BLUE).

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a type-c USB cable between the PC host and the MCU-Link port(J17 on the board).
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Expected Result
Input source and LED status table:

 PLU_IN5 | PLU_IN3 | PLU_IN2 | LED_BLUE | LED_GREEN | LED_RED
---------|---------|---------|----------|-----------|---------
 0       | 0       | 0       | off      | off       | off
 0       | 0       | 1       | off      | off       | on
 0       | 1       | 0       | off      | on        | off
 0       | 1       | 1       | on       | off       | off
 1       | 0       | 0       | off      | on        | on
 1       | 0       | 1       | on       | off       | on
 1       | 1       | 0       | on       | on        | off
 1       | 1       | 1       | on       | on        | on

The log below shows the output of the plu combination example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PLU combination driver example.

Select the input source.
1. Input source 0
2. Input source 1
3. Input source 2
4. Set all three input sources.
0
Select the input value.
0. Low level.
1. High level.
1
Select the input source.
0. Input source 0
1. Input source 1
2. Input source 2
3. Set all three input sources.
3
Input the three values like 000.
0. Low level.
1. High level.
011
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The LED will change as the description in the table
