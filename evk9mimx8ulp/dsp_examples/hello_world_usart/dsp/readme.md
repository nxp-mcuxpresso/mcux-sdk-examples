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
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============


Prepare the Demo
================
The DSP images are built into CM33 image with default project configuration.
To build the CM33 image, the DSP images dsp_reset_release.bin, dsp_text_release.bin, dsp_data_release.bin should be built firstly.

1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
This example shows two cores communicating using MU. Both M33 and DSP core run at the same time!

When the demo runs successfully, the log would be seen on the M33's debug terminal like as below, 

~~~~~~~~~~~~~~~~~~~~~~~
Hello World running on core 'Cortex-M33'

Hello World running on DSP core 'fusion_nxp02_dsp_prod'
~~~~~~~~~~~~~~~~~~~~~~~
