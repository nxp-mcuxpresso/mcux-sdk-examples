Overview
========

The srtm_demo demo application demonstrates the use of SRTM service provided in KSDK. The demo
works together with Linux kernel, providing virtual i2c service to it. Linux will utilize SRTM
APIs to configure the shared I2C. The commands are transferred via RPMSG to M core, then based on
service protocols, M core will handle the configuration to CODEC hardware.

The demo also demonstrate support for multiple partition reset.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- i.MX8QX MEK Board
- MCIMX8-8X-BB
- J-Link Debug Probe
- 12V power supply
- Personal Computer
- 2 RCA to 3.5 female audio cable

Board settings
==============
Base Board is needed to run this demo.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW3 to power on the board.
2.  Connect a micro USB cable between the host PC and the J11 USB port on the cpu board.
3.  Insert AUDIO extended card into AUDIO SLOT-1 on the base board.
4.  Using the RCA to 3.5 female cable to connect Line Out audio slots (AOUT0 and AOUT1) on the audio board and a headphone.
5.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
6.  Download the program to the target board.
7.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~
####################  CODEC SRTM DEMO ####################

    Build Time: Sep 12 2018--10:49:29

##########################################################
Press 'r' to do M4 partition reset
~~~~~~~~~~~~~~~~~~~~~

The background audio I2C service is provided for ADMA LPI2C1 and M4 LPI2C. 

Now boot up the Linux kernel with M4 specific dtb

~~~~~~~~~~~~~~~~~~~~~
setenv fdt_file 'fsl-imx8qxp-mek-rpmsg.dtb'
save
run bootcmd
~~~~~~~~~~~~~~~~~~~~~

After entering the kernel console. Use the
following command to check CS42888 playback

~~~~~~~~~~~~~~~~~~~~~
aplay -Dplughw:0 /unit_tests/ASRC/audio8k16S.wav
~~~~~~~~~~~~~~~~~~~~~

Now Insert Headphone to J8 and use the following command to check WM8960 playback

~~~~~~~~~~~~~~~~~~~~~
aplay -Dplughw:1 /unit_tests/ASRC/audio8k16S.wav
~~~~~~~~~~~~~~~~~~~~~

When kernel partition reboots (Using "reboot" command in kernel), M4 core will handle this with the following message
~~~~~~~~~~~~~~~~~~~~~
Handle Peer Core Reboot
~~~~~~~~~~~~~~~~~~~~~

On the other hand, press "r" on M4 console to trigger a M4 partition reboot when
kernel is running. WDOG timout will trigger a M4 partition reset in about 5 second. On
Linux kernel, you will observed this event is handled.

Virtual I2C service should still work fine after either kernel or M4 partition reboot.
