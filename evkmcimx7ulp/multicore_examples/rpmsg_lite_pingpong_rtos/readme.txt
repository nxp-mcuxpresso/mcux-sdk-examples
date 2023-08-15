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
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
The Multicore RPMsg-Lite pingpong RTOS project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and 
configurations in default state when running this demo.


Prepare the Demo
================
1.  Generate i.MX7ULP image file with imgutil and write to QSPI flash with U-Boot. For details, please refer to Getting Started with SDK v.2.0 for i.MX 7ULP Derivatives (Doc No: SDK20IMX7ULPGSUG)
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal for A7 core with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Open both serial lines provided at J6 USB port.
    (e.g. /dev/ttyUSB0 and /dev/ttyUSB1 on Linux or COM5 and COM6 on Windows)
5.  Flash the image for Cortex-M4 to QSPI using u-boot.
6.  Boot Linux
7.  After login, make sure imx_rpmsg_pingpong kernel module is inserted (lsmod) or insert it (modprobe imx_rpmsg_pingpong).

Running the demo
================
After the boot process succeeds, the ARM Cortex-M4 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG Ping-Pong FreeRTOS RTOS API Demo...
Link is up!
Nameservice announce sent.
Waiting for ping...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
After the Linux RPMsg pingpong module was installed, the ARM Cortex-M4 terminal displays the following information:
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
