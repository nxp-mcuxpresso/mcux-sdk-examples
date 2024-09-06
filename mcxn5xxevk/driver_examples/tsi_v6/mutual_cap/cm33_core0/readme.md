Overview
========
The tsi_v6_mutual_cap example shows how to use TSI_V6 driver in mutual-cap mode:

In this example , we make use of the available electrodes on board to show driver usage.
1. Firstly, we get the non-touch calibration results as baseline electrode counter;
2. Then, we start the Software-Trigger scan using polling method and interrupt method;
3. Then, we start the Hardware-Trigger scan using interrupt method.


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
- MCX-N5XX-EVK board
- FRDM-TOUCH board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
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
TSI_V6 Mutual-Cap mode Example Start!

Please do not touch pad KEY3 when in calibration process!
Calibrated counters for mutual-cap pad KEY3 is: 1816 

NOW, comes to the software trigger scan using polling method!
Mutual-cap pad KEY3 Normal mode counter is: 1816 

NOW, comes to the software trigger scan using interrupt method!
Mutual-cap pad KEY3 Normal mode counter is: 1928 

NOW, comes to the hardware trigger scan method!
After running, touch pad KEY3 each time, you will see LED toggles
~~~~~~~~~~~~~~~~~~~~~
