Overview
========
This demo is used to show all the power modes in core 0 (the master core in dual_core SoC). With this demo, user can navigate all the power modes that are existing in core 0.
In typical use case, the master core is running the main control logic, while the slave core is used to run the protocol or other complex mid-ware independently.

Then the slave core prepares the resource (data, message, etc) into the somewhere, so that the master core would fetch when necessary. In some other cases, master core would send the some kinds of message to the slave core, and ask the slave core to enter indicated power mode. This demo download the image to both core, also provides the reference code to enter any power mode.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- FRDM-K32L3A6 board
- Personal Computer

Prepare the demo
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Set the SoC booting up from core 0 (CM4 core), since the program would be downloaded and run in core 0.
4.  Build the core 1's image first and build the core 0's image with core 1's image linked inside.
5.  Download the program to the target board.
6.  Press the reset button on your board to begin running the demo.
7.  Don't debug dual core at the same time, because cm0plus is in VLLS0 and could not be
    connected.

Running the demo
================
In this demo, cm4 core is used as the master core, which would boot from reset. The cm0plus core would be awaken up when cm4 core
executes it. Then the cm0plus core would enter the VLLS0 mode.
When the demo runs successfully, the log would be seen on the OpenSDA terminal like as below and LED will blink:

~~~~~~~~~~~~~~~~~~~~~

Power Mode Switch demo - dual core, core 0.
Boot up another core, core 1 ...
Core 1 is booted up now.
Shake hands done.

**************************************************************************
    Power mode: RUN
--------------------------------
 - A: kAPP_PowerModeRun
 - B: kAPP_PowerModeWait
 - C: kAPP_PowerModeStop
 - D: kAPP_PowerModeVlpr
 - E: kAPP_PowerModeVlpw
 - F: kAPP_PowerModeVlps
 - G: kAPP_PowerModeLls
 - H: kAPP_PowerModeVlls2
 - I: kAPP_PowerModeVlls0
 - J: kAPP_PowerModeHsrun
Input A to J:
~~~~~~~~~~~~~~~~~~~~~
