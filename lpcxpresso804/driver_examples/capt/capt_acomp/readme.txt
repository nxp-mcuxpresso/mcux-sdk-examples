## Overview
-----------
This example shows how to capture touch data using CAPT poll-now mode.
Comparing with the capt_basic example, this example uses ACOMP
instead of GPIO in measurement stage.
In this mode, application trigger the polling round, after polling round, the
CAPT stops and notifies application by interrupt or register status.
In this example, application poll the enabled pins one by one, after all pins
are sampled, the result are shown in the debug terminal. Then application
starts to sample all the enabled pins again.

When the example runs, the sampled data is shown in the terminal. When electrode
plate is touched, the sampled data changes too.

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
Short R6 on CapTouch Shield board

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
The CAPT sample data is shown in the terminal, touch the electrode plate to see
sample data change.
