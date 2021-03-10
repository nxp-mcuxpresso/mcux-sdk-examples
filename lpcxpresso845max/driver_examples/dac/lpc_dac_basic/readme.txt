## Overview
-----------
The dac_basic example shows how to use DAC module simply as the general DAC converter.

When the DAC's double-buffer feature is not enabled, the CR register is used as the DAC output data register directly.
The converter would always output the value of the CR register. In this example, it gets the value from terminal,
outputs the DAC output voltage through DAC output pin.

## Functional Description
-------------------------
In this example, The DAC will output the voltage received from serial terminal.

When start running this example, main routine will initialize clock, pin mux configuration,
and configure the DAC module to make it work in basic mode. Users are prompted to 
input DAC value in serial terminal, board will outputs the DAC output voltage through DAC
output pin.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso845MAX board
- Personal Computer

## Board Settings
------------------------
J6-5 is DAC0 output pin.

## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the 
   LPCXpresso845MAX board).

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running the project.

5. Input several characters according to prompt information displayed in terminal. 

## Expected Result
------------------------
Users input the value via serial terminal, then board outputs the DAC output voltage
through DAC output pin(J6-5). 
