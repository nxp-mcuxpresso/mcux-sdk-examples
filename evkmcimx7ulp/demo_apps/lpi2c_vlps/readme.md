Overview
========
This example describes how to use kinetis SDK drivers to implement LPI2C transmit and receive in the VLPS (very low power STOP mode) with async DMA.
The LPI2C module is designed to have ability to work under low power module like STOP, VLPW and VLPS. It can use DMA to transmit the data from or to application user buffer without CPU interaction.

It uses LPI2C to access the on board accelerometer sensor to read the Accelerometer X, Y, Z data every 500ms. CPU would keep in VLPS low power mode, except for some trigger events and data output to LPUART0.

SDK version
===========
- Version: 2.15.0

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
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
This should be run together with uboot.
1.  Build the project and generate the .bin file.
2.  Copy the .bin file to <sdk_dir>/tools/imgutil/evkmcimx7ulp folder and rename
    to sdk20-app.bin.
3.  In the folder <sdk_dir>/tools/imgutil/evkmcimx7ulp, run "mkimage.sh ram or mkimage.sh flash" 
    (see readme in the <sdk_dir>/tools/imgutil )to create bootable image.
4.  Prepare an SD card with the prebuilt U-Boot image and copy the sdk20-app.img
    generated into the SD card. Then, insert the SD card to the target board.
    Make sure to use the default boot SD slot and check the dip switch configuration
5.  Open a serial terminal with the following settings to connect to Cortex-A:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
6.  Connect 5V power supply to the board, switch SW1 to power on the board.
7.  Hit any key to stop autoboot in the terminals, then enter to U-Boot command
    line mode. Then write the image and run it from QSPI Flash with the following
    commands:
    - sf probe
    - sf erase 0x0 0x20000
    - fatload mmc 0:1 0x62000000 sdk20-app.img
    - sf write 0x62000000 0x0 0x20000
8.  Open a serial terminal with the following settings to connect to Cortex-M:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
9.  Switch the SW1 to re-power the board.
10. Hit any key to stop uboot autoboot in the Cortex-A terminals.

Running the demo
================
Disconnect any debugger to run this demo in case the sequence of entering sleep mode is interrupted.
When the example runs successfully, you can see the similar information from the terminal as below.
~~~~~~~~~~~~
LPI2C_VLPS demo start...
Enter VLPS mode aborted!
X:    2,  Y: -148,  Z: 2107
X:   -2,  Y:  -97,  Z: 2086
~~~~~~~~~~~~
