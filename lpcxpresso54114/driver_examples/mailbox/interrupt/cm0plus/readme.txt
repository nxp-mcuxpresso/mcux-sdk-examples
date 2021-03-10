Overview
========
The mailbox_interrupt example shows how to use mailbox to exchange message.

In this example:
The primary core writes a value to the secondary core mailbox, it causes mailbox interrupt
on the secondary core side. The secondary core reads the value from mailbox, it increments and writes it to mailbox register
for the primary core, which causes mailbox interrupt on the primary core side. The primary core reads value from mailbox, 
it increments and writes it to the mailbox register for the secondary core again.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54114 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J7) on the board
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
Copy CORE1 image to address: 0x20010000, size: 2522
Write to CM0+ mailbox register: 1
Read value from CM4 mailbox register: 2
Write to CM0+ mailbox register: 3
Read value from CM4 mailbox register: 4
Write to CM0+ mailbox register: 5
Read value from CM4 mailbox register: 6
Write to CM0+ mailbox register: 7
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
