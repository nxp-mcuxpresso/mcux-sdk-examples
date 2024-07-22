Overview
========
The LPCMP round robin example is a simple demonstration program that uses the LPCMP driver to
help users get started quickly. In this example, users need to specify which port and channel
to fix, and which channels to select as checker channels. Before doing the comparison, it is
necessary to preset the comparison result of each checker channel with the fixed channel. 
If any results of the comparator are different from the preset value, an interrupt will be 
triggered, and the information of the triggered channel will be printed on the terminal after
changing the input voltage of checker channels.

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
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name    Board Location     checker channel number       pre-set value           trigger        
P2_2            J2-9                channel 0                    1              input > DAC output
P1_3            J6-3                channel 1                    0              input < DAC output
P1_4            J1-2                channel 2                    1              input > DAC output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example uses LPCMP0 instance, the fixed port is plusMuxPort, the fixed channel is
channel 7(Internal DAC module), and the checker channels are channel 0(CMP0_IN0, P2_2, J2-9),
channel 1(CMP0_IN1, P1_3, J6-3) and channel 2(CMP0_IN2, P1_4, J1-2), in miusMuxPort.
The LPCMP's internal DAC module output voltage signal(half of VREFI), the channel's pre-set value
is 0x05U.

Set the input voltage of channel 0, channel 1 and channel 2 to be greater than, less than
and greater than the output voltage of the DAC respectively. The comparison results of the 
three channels are different from the pre-set value. So the interrupt occurred and the terminal
will print corresponding channel changed information.

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
When the demo runs successfully, following information can be seen on the OpenSDA terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPCMP RoundRobin Example.
channel 0 comparison result is different from preset value!
channel 1 comparison result is different from preset value!
channel 2 comparison result is different from preset value!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
