## Overview
-----------
This example shows how to use CAPT to detect key touch event.
When the key is touched, LED on the board is on accordingly.
The example first runs the calibration, in this period, all LEDs
are on, please don't touch any key at this time. After all LEDs off
the example starts touch detection, touch any key and the LED turns on.
This example only supports touch one key at the same time.
CAPT sample data are saved in a window for later process.
During calibration period, the variance of each channel is used to judge
whether the channel is stable. Calibration finished only when all channels
are stable. The stable value after calibration is saved as baseline, to
be used in the detection stage.
To remove touch glitch, a software filter is used in this example.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso804 board
- CapTouch Shield board
- Personal Computer

## Board Settings
------------------------
Remove JP3, JP21, JP22, JP23, JP25 and C13 on LPCXpresso804 board.

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the 
   LPCXpresso804 board) for receiving debug information.

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Start the slave board on another board first, then launch the debugger in your IDE to
   begin running this project.

5. Monitor the information on the debug console.

## Expected Result
------------------------
When touch the electrode plate, the corresponding LED would turn on.
