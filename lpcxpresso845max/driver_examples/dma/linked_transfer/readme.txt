## Overview
-----------------------------------------------------------------------------------------
The DMA linked example is a simple demonstration program that uses the SDK software.
It executes linked transfer(ping pong case) from the source buffer to destination buffer using 
the SDK DMA drivers.

Two descriptors are used,  A is linked to B, B is linked to A.
```
A------>B------->
^               |
|               |
---------<-------
```
The purpose of this example is to show how to use the DMA and to provide a simple example for
debugging and further development.

## Functional description
-----------------------------------------------------------------------------------------
This example shows the usage of more than one descriptors to define a complicated transfer.

## Toolchain supported
---------------------
- IAR embedded Workbench 8.50.5
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
~~~~~~~~~~~~~~~~~~~~~
DMA linked transfer example begin.

Destination Buffer:

0   0   0   0   0   0   0   0

DMA linked transfer example finish.

Destination Buffer:

1   2   3   4   11  22  33  44
~~~~~~~~~~~~~~~~~~~~~
