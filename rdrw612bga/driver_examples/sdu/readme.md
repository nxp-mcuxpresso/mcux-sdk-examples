Overview
========

The sdu example shows how to use sdu driver to exchange command/data with host driver.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
rdrw612bga as the sdio device, need to run with a sdio host, could use evkbmimxrt1060 as the host.
Connect rdrw612bga with evkbmimxrt1060 with sdio cable.
Set JP16 to 1.8V for rdrw612bga board.

Prepare the Demo
================
On host side, ncp_host with loopback mode offers a loopback test cmd for users to test SDIO loopback.
On device side, sdu_example app offers a simple SDIO interface example which corresponding with SDIO interface loopback test.
Loopback mode means host side generate packet and send to device side, after the packet is received by device side, a packet with the same content will be send back to host side. It is only a test mode for SDIO interface.

For rdrw612bga device side:
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

For evkbmimxrt1060 host side:
1. Build ncp_host example
   Define macro ‘CONFIG_NCP_SDIO 1’ in ncp_host_config.h.
   Define macro ‘COFNIG_NCP_SDIO_TEST_LOOPBACK 1’ in ncp_host_config.h.
   Define macro ‘CONFIG_NCP_UART 0’ in ncp_host_config.h.
   Define macro ‘CONFIG_NCP_SPI 0’ in ncp_host_config.h.
   Define macro ‘CONFIG_NCP_USB 0’ in ncp_host_config.h.
2. Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
3. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Take evkbmimxrt1060 as host side for example.
Device side: sdu_example(rdrw612bga)
Host side: ncp_host(evkbmimxrt1060)

Usage:
        test-loopback <string1> <string2> ... <stringN>

Example: (Input test-loopback commands on host side)
# test-loopback hello 1234567890 abc

# Response test-loopback cmd size=0x1e:
**** Dump @ 20006C48 Len: 30 ****
01 00 01 04 1e 00 00 00 00 00 00 00 68 65 6c 6c
6f 31 32 33 34 35 36 37 38 39 30 61 62 63
******** End Dump *******
