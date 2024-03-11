Overview
========
This example shows how to use CTimer to capture the edge. In this example, CTimer
timer counter uses APB clock as clock source, and CTimer monitors capture pin.
The capture pin is connected to a GPIO output. The project pulls GPIO to generate
rising edge. When rising edge detected on the pin, CTimer saves the timer counter value
to capture register, and print in the terminal. The GPIO pin is toggled priodically,
so the edge capture timestamp is shown periodically in the terminal.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
Short HD2 Pin 6 and Pin 7

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The CTimer captured rising edge timestamp is print in the terminal periodically.
