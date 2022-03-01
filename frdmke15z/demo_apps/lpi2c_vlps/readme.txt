Overview
========
This example describes how to use kinetis SDK drivers to implement LPI2C transmit and receive in the VLPS (very low power STOP mode) with async DMA.
The LPI2C module is designed to have ability to work under low power module like STOP, VLPW and VLPS. It can use DMA to transmit the data from or to application user buffer without CPU interaction.

It uses LPI2C to access the on board accelerometer sensor to read the Accelerometer X, Y, Z data every 500ms. CPU would keep in VLPS low power mode, except for some trigger events and data output to LPUART0.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE15Z board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Press the reset button to begin running the demo.

Running the demo
================
Disconnect any debugger to run this demo in case the sequence of entering sleep mode is interrupted.
When the example runs successfully, you can see the similar information from the terminal as below.
~~~~~~~~~~~~
LPI2C_VLPS demo start...
X:    2,  Y: -148,  Z: 2107
X:   -2,  Y:  -97,  Z: 2086
~~~~~~~~~~~~
