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
8. After login Linux, make sure imx_rpmsg_tty kernel module is inserted (lsmod) or install it (modprobe imx_rpmsg_tty).

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
echo test > /dev/ttyRPMSG30 (For Cortex-M4 Core0)
echo test > /dev/ttyRPMSG31 (For Cortex-M4 Core1)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
On the M4 terminal, the received string content and its length is output, as shown in the log.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get Message From Master Side : "test" [len : 4]
Get New Line From Master Side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
