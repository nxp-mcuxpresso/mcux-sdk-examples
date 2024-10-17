Overview
========
The LPCMP polling Example shows the simplest way to use LPCMP driver and help user with a quick start.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
LPCMP's positive channel input. On the negative side, the internal DAC is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user's voltage crosses the internal DAC's value. The endless loop in main() function
would detect the logic value of comparator's output, and change the LED. The LED would be turned on when the compare
output is logic one, or turned off when zero.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name       Board Location     Pin Name    Board Location
VREFI/P2_7         J3-1           VDD         JP8-2
CMP0_IN0/P2_2      J2-9           VDD or GND
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example project uses LPCMP0 to compare the voltage signal input from CMP0_IN0(P2_2, J2-9)
with the voltage signal(half of VREFI) output by LPCMP's internal DAC.
Connect J3-1(P2_7,VREFI) to JP8-2(VDD), this will set the VREFI to VDD(The VREFI is used as LPCMP reference voltage).
Change the IO state of J2-9(P2_2, CMP0_IN0) will toggle the RED LED.

Prepare the Demo
================
1. Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
2. Open a serial terminal on PC for the serial device with these settings:
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPCMP Polling Example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

