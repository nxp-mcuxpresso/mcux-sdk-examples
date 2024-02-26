Overview
========
The FreeRTOS Message Buffers multicore project is a simple demonstration program that uses the
MCUXpresso SDK software and the Message Buffers component of FreeRTOS. It shows how to implement 
lightweight core to core communication using FreeRTOS Message Buffers, which are lockless circular buffers 
that can pass data packets of varying sizes from a single sender to a single receiver.
The primary core releases the secondary core from the reset and then the inter-core communication 
is established then. Once the Message Buffers are initialized on both cores the message exchange starts,
incrementing a virtual counter that is part of the message payload. The message pingpong finishes 
when the counter reaches the value of 100. The Message Buffer is deinitialized at the end.
NXP Multicore Manager (MCMGR) component is used for several purposes in this example:
- it releases the secondary core from the reset (starts the secondary code)
- it registers and use the application event for init handshaking (the secondary core application 
  signals to the primary core it is ready to communicate)
- it registers and use the FreeRtosMessageBuffersEvent for inter-core interrupts triggering and
  interrupts handling  

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application to allow placing FreeRTOS Message Buffers
at a fixed location that is then known for both cores.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N5XX-EVK board
- Personal Computer

Board settings
==============
The FreeRTOS Message Buffers multicore project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and 
configurations in default state when running this demo.


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
The log below shows the output of the FreeRTOS Message Buffers multicore demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS Message Buffers demo starts

Copy CORE1 image to address: 0x2004e000, size: 15548
Primary core received a msg
Message: Size=4, DATA = 1
Primary core received a msg
Message: Size=4, DATA = 3
Primary core received a msg
Message: Size=4, DATA = 5
.
.
.
Primary core received a msg
Message: Size=4, DATA = 101

FreeRTOS Message Buffers demo ends

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
