Overview
========
The Multicore RPMsg-Lite pingpong RTOS project is a simple demonstration program that uses the
MCUXpresso SDK software and the RPMsg-Lite library and shows how to implement the inter-core
communicaton between cores of the multicore system. The primary core releases the secondary core
from the reset and then the inter-core communication is established. Once the RPMsg is initialized
and endpoints are created the message exchange starts, incrementing a virtual counter that is part
of the message payload. The message pingpong finishes when the counter reaches the value of 100.
Then the RPMsg-Lite is deinitialized and the procedure of the data exchange is repeated again.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8QX MEK board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
The Multicore RPMsg-Lite pingpong RTOS project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and 
configurations in default state when running this demo.


Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW3 to power on the board.
2.  Connect a micro USB cable between the host PC and the J11 USB port on the cpu board.
3.  Open 2 serial terminals with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  One terminal is for A35, the other is for M4. Refer to Getting Started with MCUXpresso SDK for i.MX 8QuadXPlus(Doc No.: MCUXSDKIMX8QXGSUG) for more COM port information.
5.  Download the M4 side program to the target board.
6.  Launch the debugger in your IDE to begin running the M4 side example.
7.  Boot Linux in A35 Core.
8.  Install imx_rpmsg_pingpong.ko kernel module in Linux side to kick off the demo.

Running the demo
================
After the boot process succeeds, the ARM Cortex-M4 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG Ping-Pong FreeRTOS RTOS API Demo...
RPMSG Share Base Addr is 0xb8000000
Link is up!
Nameservice announce sent.
Waiting for ping...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
After the Linux RPMsg pingpong module(imx_rpmsg_pingpong.ko) was installed, the ARM Cortex-M4 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sending pong...
Waiting for ping...
Sending pong...
Waiting for ping...
Sending pong...
......
Waiting for ping...
Sending pong...
Ping pong done, deinitializing...
Looping forever...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The Cortex-A terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
get 1 (src: 0x1e)
get 3 (src: 0x1e)
......
get 99 (src: 0x1e)
get 101 (src: 0x1e)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
