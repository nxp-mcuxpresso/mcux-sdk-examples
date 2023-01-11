## Overview
-----------------------------------------------------------------------------------------
The DMA hardware trigger example is a simple demonstration program that
uses the SDK software.It executes one shot transfer from source buffer to
destination buffer using the SDK DMA drivers by hardware trigger.The purpose of 
this example is to show how to use the DMA and to provide a simple example 
fordebugging and further development.

## Functional description
-----------------------------------------------------------------------------------------
This example shows the basic usage for DMA. Just move data from source address located in RAM to
destination address which also located in RAM by hardware trigger.

## Toolchain supported
---------------------
- IAR embedded Workbench 9.30.1
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Mini/micro USB cable
- LPCXpresso845MAX board
- Personal Computer

## Board Settings
------------------------
No special settings are required.

## Run the Demo
------------------------
1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the board).

2. Open a serial terminal with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Choose an IDE, building the project and download the program to the target board.
   More information about how to compile and program the project can refer to the 

   [Getting Started with MCUXpresso SDK](../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running the demo.

## Expected Result
------------------------
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DMA hardware trigger example begin.

Destination Buffer:
0	0	0	0	

Press SW1 to trigger one shot DMA transfer.

SW1 is pressed.

DMA hardware trigger example finish.

Destination Buffer:
1	2	3	4	
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
