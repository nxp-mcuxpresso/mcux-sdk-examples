Overview
========
The ELE common application provides example how to load loadable FW into ELE and how to distingush when FW is loaded.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Personal Computer
- KW45B41Z-EVK board

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
ELE Common example
Get ELE FW version
FW version[0]: FFFFFFFF, version[1]: FFFFFFFF
FW is not loaded in ELE
Load ELE FW
FW version[0]: 1010000, version[1]: 4867EE6D
FW is loaded in ELE
Example end
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

