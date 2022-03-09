Overview
========
The Simple Match Interrupt project is to demonstrate usage of the SDK CTimer driver with interrupt callback functions
In this example the upon match and IO pin connected to the LED is toggled and the timer is reset, so it would generate a square wave.
With an interrupt callback the match value is changed frequently in such a way that the frequency of the output square wave is increased gradually.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
The log below shows example output of the CTimer simple match demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer match example to toggle the output.
This example uses interrupt to change the match period.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Observe the green led to see the ctimer toggle operation.
