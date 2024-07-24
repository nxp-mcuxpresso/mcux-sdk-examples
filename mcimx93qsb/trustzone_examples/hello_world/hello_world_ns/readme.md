Overview
========
The Hello World demo application provides a sanity check for the new SDK build environments and board bring up. This demo application also utilizes TrustZone, 
so it demonstrates following techniques for TrustZone applications development:
1. Application separation between secure and non-secure part
2. TrustZone environment configuration
3. Exporting secure function to non-secure world
4. Calling non-secure function from secure world
4. Creating veneer table
5. Configuring IAR, MDK, GCC and MCUX environments for TrustZone based projects

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-QSB board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the flash.bin to the target board.
    Reference 'How to get flash.bin of TrustZone examples'
    Reference 'Getting Started with MCUXpresso SDK for MCIMX93-QSB.pdf' to make and download flash.bin.
5.  Press the reset button on your board.

How to get flash.bin of TrustZone examples:
1.  Create a directory named 'tz_bin' in your work direcory.
2.  Build {app}_s and copy sdk20-app.bin into 'tz_bin' then rename it as sdk20-app_s.bin.
3.  Build {app}_ns and copy sdk20-app.bin into 'tz_bin' then rename it as sdk20-app_ns.bin.
4.  Enter the 'tz_bin' directory and execute 'dd' command to combin the two files.
    dd if=sdk20-app_ns.bin of=sdk20-app_s.bin bs=1 count=`ls -l ./sdk20-app_ns.bin|awk '{print $5}'` seek=65536
5.  Rename sdk20-app_s.bin as m33_image.bin, and copy it into imx-mkimage
6.  Build the low power boot flash.bin

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!
This is a text printed from normal world!
Comparing two string as a callback to normal world
String 1: Test1
String 2: Test2
Both strings are not equal!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
