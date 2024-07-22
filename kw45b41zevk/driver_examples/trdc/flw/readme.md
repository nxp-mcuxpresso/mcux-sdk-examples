Overview
========
The trdc flw example shows how to use the trdc flash logical window funtion.

In this example, flash logical window is configured and enabled. First memory
outside the window is touched and the hardfault occurs, then enlarge the window
size to make the access legal. Next the data mapping between the physical address
and programmable flash address is checked.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- KW45B41Z-EVK Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TRDC flash logical window example start
Set the flash logical window
Violent access at address: 0x 1080000
The flash memory is accessiable now
The data between physical address window and programmable flash address are identical.
TRDC flash logical window example success
~~~~~~~~~~~~~~~

