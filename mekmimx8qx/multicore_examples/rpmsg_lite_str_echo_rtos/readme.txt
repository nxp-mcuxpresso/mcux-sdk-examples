Overview
========
The Multicore RPMsg-Lite string echo project is a simple demonstration program that uses the
MCUXpresso SDK software and the RPMsg-Lite library and shows how to implement the inter-core
communicaton between cores of the multicore system.

It works with Linux RPMsg master peer to transfer string content back and forth. The name service
handshake is performed first to create the communication channels. Next, Linux OS waits for user
input to the RPMsg virtual tty. Anything which is received is sent to M4. M4 displays what is
received, and echoes back the same message as an acknowledgement. The tty reader on the Linux side
can get the message, and start another transaction. The demo demonstrates RPMsg’s ability to send
arbitrary content back and forth. Note: The maximum message length supported by RPMsg is now 496
bytes. String longer than 496 will be divided by virtual tty into several messages.

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
8.  Install imx_rpmsg_tty.ko kernel module in Linux side to kick off the demo.

Running the demo
================
After the boot process succeeds, the ARM Cortex-M4 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG String Echo FreeRTOS RTOS API Demo...

Nameservice sent, ready for incoming messages...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
After the Linux RPMsg tty module was installed, the ARM Cortex-M4 terminal displays the following
information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get Messgae From Master Side : "hello world!" [len : 12]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The user can then input an arbitrary string to the virtual RPMsg tty using the following echo command on
Cortex-A terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo test > /dev/ttyRPMSG30 log below shows the output of the RPMsg-Lite str echo demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
On the M4 terminal, the received string content and its length is output, as shown in the log.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get Message From Master Side : "test" [len : 4]
Get New Line From Master Side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
