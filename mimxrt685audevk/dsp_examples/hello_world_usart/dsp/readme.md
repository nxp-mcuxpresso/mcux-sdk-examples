Overview
========
The dsp_hello_world_usart demo application provides a sanity check for the SDK build
environment running both ARM and DSP cores. It also demonstrates how the ARM 
core application is used to start a DSP application.

This demo contains two applications:
- cm33/ is the ARM application for Cortex-M33 core
- dsp/ is the DSP application for HiFi4 core

The release configurations of the demo will combine both applications into one ARM
image.  With this, the ARM core will load and start the DSP application on
startup.  Pre-compiled DSP binary images are provided under dsp/binary/ directory.

The debug configurations will build two separate applications that need to be
loaded independently.  The ARM application will power and clock the DSP, so
it must be loaded prior to loading the DSP application.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- Xtensa C Compiler  14.01

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the debug USB port (J5) on the board
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program for CM33 core to the target board.
4.  Launch the debugger in your IDE to begin running the demo.
5.  If building debug configuration, download the program for DSP core to the target board.
6.  If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to
begin running the demo.

NOTE: DSP image can only be debugged using J-Link debugger.  See
'Getting Started with Xplorer for MIMXRT685-AUD-EVK.pdf' for more information.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:

    Hello World running on core 'Cortex-M33'

Running the demo DSP
====================
When the demo runs successfully, the terminal will display the following:

    Hello World running on DSP core 'nxp_rt600_RI2020_5_newlib'

