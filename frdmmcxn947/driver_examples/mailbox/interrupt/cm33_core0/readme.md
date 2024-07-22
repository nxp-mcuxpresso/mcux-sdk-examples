Overview
========
The mailbox_interrupt example shows how to use mailbox to exchange message.

In this example:
The primary core writes a value to the secondary core mailbox, it causes mailbox interrupt
on the secondary core side. The secondary core reads the value from mailbox, it increments and writes it to mailbox register
for the primary core, which causes mailbox interrupt on the primary core side. The primary core reads value from mailbox, 
it increments and writes it to the mailbox register for the secondary core again.

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
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J17) on the board
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
The log below shows example output of the Mailbox interrupt example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Mailbox interrupt example
Copy CORE1 image to address: 0x2004e000, size: 6940
Write to the secondary core mailbox register: 1
Read value from the primary core mailbox register: 2
Write to the secondary core mailbox register: 3
Read value from the primary core mailbox register: 4
Write to the secondary core mailbox register: 5
Read value from the primary core mailbox register: 6
Write to the secondary core mailbox register: 7
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
