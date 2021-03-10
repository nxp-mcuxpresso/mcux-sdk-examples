## Overview
-----------------------------------------------------------------------------------------
The DMA channel chain transfer example is a simple demonstration program about how to use channel 
chain feaute with the SDK software.
In this example three channel is used, 
```
          ------->channel1------->channel0
          |
channel0-->
          |
          ------->channel2
```
Channel0 is configured with two descriptor, first descriptor is linked to the second, then trigger 
the channel0 by software, after channel0 first descriptor finish, it will trigger channel1 and 
channel2 start transfer, after channel1 descriptor exhaust it will trigger channel0, then channel0 
second descritpor will be carried out, after second descriptor exhaust, example finish.

## Functional description
-----------------------------------------------------------------------------------------
Channel chaining is a feature, which allows completion of a DMA transfer on channel x to trigger a 
DMA transfer on channel y.

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
DMA channel chain example begin.
Destination Buffer:
0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0
DMA channel chain example finish.
Destination Buffer:
1	2	3	4	11	22	33	44 111 222 333 444	1111 2222 3333 4444
~~~~~~~~~~~~~~~~~~~~~
