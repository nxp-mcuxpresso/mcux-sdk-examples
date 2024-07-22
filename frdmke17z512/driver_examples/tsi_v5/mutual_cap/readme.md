Overview
========
The tsi_v5_mutual_cap example shows how to use TSI_V5 driver in mutual-cap mode:

In this example , we make use of the available electrodes on board to show driver usage.
1. Firstly, we get the non-touch calibration results as baseline electrode counter;
2. Then, we start the Software-Trigger scan using polling method and interrupt method;
3. Then, we start the Hardware-Trigger scan using interrupt method.


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
- FRDM-KE17Z512 board
- FRDM-TOUCH board
- Personal Computer

Board settings
==============
Keep the default configuration for the FRDM board and connect the FRDM-TOUCH board with the FRDM board.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the MCU-LINK USB port on the board.
2.  Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~
