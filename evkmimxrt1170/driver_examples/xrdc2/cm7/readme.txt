Overview
========
The xrdc2 example shows how to control the memory slot, memory region, and
peripheral access policy using XRDC2.

In this example, the memory slot, memory region, and peripheral are set to
unaccessible. Then access to them triggers hardfault or bus fault. In the
exception handler, theses resources are set to accessible, then the previous
access continues and success.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the demo
1.  Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build core1 project first, then build core0 project.
4.  Download the core0 program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the success log "XRDC2 example Success" is
shown in the terminal.
