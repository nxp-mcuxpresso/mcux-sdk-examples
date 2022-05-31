Overview
========
In this demo, A core decoded music data and put it to DDR buffer and informs M core with the related information.
Then M core will take the ownership of consuming the buffer, it will copy buffer from DDR to TCM, manipulating SDMA to transfer the data to SAI and codec for playback.
It gives DDR and A core opportunity to do power saving for rather long time frame. M core will also take ownership of codec initialization.
SRTM(Simplified Real Time Messaging) protocol is used to communicate between A core and M core.
The protocol provides various commands for A core and M core to communicate with each other.
If there is no audio palyback, M core will enter the STOP mode, and the whole SOC system would enter deep sleep mode(DSM) once A core enter low power status.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8MM6-EVK  board
- J-Link Debug Probe
- 12V power supply
- Personal Computer
- Headphone

Board settings
==============
No special settings are required.

#### Note! ####
1.  This case does not support ddr and flash target.
2.  This case runs together with Linux and the Linux release version should be not lower than 5.10.72-2.2.0.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

******************
NOTE
******************
1.  After M core running, please boot the linux kernel to create the rpmsg channel between A core and M core.
    Make sure the FDT file is correctly set before booting the linux kernel. The following command can be used to set FDT file in uboot console:
	u-boot=>setenv fdtfile imx8mm-evk-rpmsg.dtb
    u-boot=>saveenv

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
####################  RPMSG UART SHARING DEMO  ####################

    Build Time: May 31 2022--20:19:21
    ********************************
     Wait the Linux kernel boot up to create the link between M core and A core.

    ********************************
    The rpmsg channel between M core and A core created!
    ********************************


    Task A is working now.
