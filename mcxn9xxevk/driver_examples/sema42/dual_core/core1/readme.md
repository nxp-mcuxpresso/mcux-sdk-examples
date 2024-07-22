Overview
========
The sema42 example shows how to use SEMA42 driver to lock and unlock a sema gate:

In this example:
1. Firstly, Core 0 turn on LED  and lock a sema gate then boot up Core 1 wake up.
2. Core 1 must be wait until Core 0 unlocks this sema gate to lock it.
3. After user press any key in terminal window, the sema gate will be unlocked by core 0,
then core 1 will lock it and turn off the LED

If the board does not have LED to show the status, then core 1 will send a flag
to core 0 when core 1 has locked the sema gate. Core 0 outputs the success log
after it received the flag.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like below
and Red LED will turn on.

~~~~~~~~~~~~~~~~~~~~~
Sema42 example!

Press any key to unlock semaphore and Core 1 will turn off the LED
~~~~~~~~~~~~~~~~~~~~~

When press any key in the terminal, the LED will be turned off and the terminal tells

~~~~~~~~~~~~~~~~~~~~~
Now the LED should be turned off

Sema42 example succeed!
~~~~~~~~~~~~~~~~~~~~~
