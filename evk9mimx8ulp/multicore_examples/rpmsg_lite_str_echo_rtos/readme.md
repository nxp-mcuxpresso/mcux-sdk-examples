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

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
The Multicore RPMsg-Lite string echo project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and
configurations in default state when running this demo.


Prepare the Demo
================
1.  Generate i.MX8ULP image file with imx-mkimage and write to emmc/flexspi0 nor flash with U-Boot/UUU/JLink. For details, please refer to Getting Started with SDK v.2.0 for i.MX 8ULP Derivatives (Doc No: SDK20IMX8ULPGSUG)
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open two serial terminals for A Core and M Core with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
    (e.g. /dev/ttyUSB0~3, /dev/ttyUSB2 for A Core, /dev/ttyUSB3 for M Core)
4.  Flash the image to emmc/flexspi0 nor flash using u-boot/UUU/JLink.(Don't use IAR/GDB to download m33 binary to SSRAM to test it)
5.  Repower on board to boot system
6.  Login to linux
7.  After login, make sure imx_rpmsg_tty kernel module is inserted (lsmod) or insert it (modprobe imx_rpmsg_tty).

Running the demo
================
After the boot process succeeds, the ARM Cortex-M33 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RPMSG String Echo FreeRTOS RTOS API Demo...

Nameservice sent, ready for incoming messages...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
After the Linux RPMsg tty module was installed, the ARM Cortex-M33 terminal displays the following
information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get Messgae From Master Side : "hello world!" [len : 12]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The user can then input an arbitrary string to the virtual RPMsg tty using the following echo command on
Cortex-A terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo test > /dev/ttyRPMSG<num>
<num> here is the allocated ttyRPMsg channel number. Please find out the number in the file system by "ls" command.
Log below shows the output of the RPMsg-Lite str echo demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
On the M33 terminal, the received string content and its length is output, as shown in the log.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Get Message From Master Side : "test" [len : 4]
Get New Line From Master Side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
