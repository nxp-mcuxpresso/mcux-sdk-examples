Overview
========
Application demonstrating how to use the a2dp source feature.


Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1170 board
- Personal Computer
- Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
- Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.

Jumper settings for RT1170-EVKB (enables external 5V supply):
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

Murata Solution Board settings
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1ZM M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.

Note:
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW4 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Provide 5V voltage for the target board.

4.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

5.  Download the program to the target board.

6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USB Host stack successfully initialized
Bluetooth initialized

SHELL build: Mar  2 2021
Copyright  2021  NXP

>> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list
2 input "bt discover" to discover connctable bluetooth devices
3 input "bt connect [index]" to create basic bluetooth connection with the discovered device
4 the music start playing after connection success.
