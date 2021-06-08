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
- i.MX8QM MEK CPU Board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
No special settings are required.

This multicore example uses the shared memory for data exchange. The shared memory region start address should be same
for the two cores exchanging data. The start address is defined by "#define RPMSG_LITE_SHMEM_BASE 0x90010000" in main_remote.c for Cortex-M4 core0,
"#define RPMSG_LITE_SHMEM_BASE 0x90110000" for Cortex-M4 core1.
The Linux kernel need boot with RPMSG specific dtb.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board (Refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for debug port information).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4. Open Cortex-A core debug console. Please refer to Getting Started with MCUXpresso SDK for i.MX 8QuadMax(Doc No.: MCUXSDKIMX8QMGSUG) for COM port information.
5. Download the program to the target board (Please refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for how to run different targets).
6. Run the following commands in Uboot console to set Linux kernel boot with RPMSG dtb.
     =>setenv fdt_file 'fsl-imx8qm-mek-rpmsg.dtb'
     =>save
     =>run bootcmd
7. Boot Linux
8. After login Linux, make sure imx_rpmsg_pingpong kernel module is inserted (lsmod) or insert it (modprobe imx_rpmsg_pingpong).

Running the demo
================
This examples shows how to implement the inter-core communicaton between Cortex-M4 core and Cortex-A core using rpmsg-lite library.
After the boot process succeeds, the ARM Cortex-M4 core0 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG Ping-Pong FreeRTOS RTOS API Demo...
RPMSG Share Base Addr is 0x90010000
Link is up!
Nameservice announce sent.
Waiting for ping...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
For Cortex-M4 core1 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG Ping-Pong FreeRTOS RTOS API Demo...
RPMSG Share Base Addr is 0x90110000
app_srtm: AUTO and I2C service registered
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
The Cortex-A terminal displays the following similar information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
get 1 (src: 0x1e)
get 3 (src: 0x1e)
......
get 99 (src: 0x1e)
get 101 (src: 0x1e)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
