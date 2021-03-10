## Overview
-----------
This example demonstrates the use of the USART module in dma-driven asynchronous mode to 
receive characters from a terminal emulator. USART will echo back every 8 characters to 
terminal emulator.

## Functional Description
-------------------------
In this example, USART communicates with a terminal emulator running on the PC, via 
the VCOM serial port. 

1. Once the project start, main routine will initialize clock, pin mux configuration, 
   configure the USART module and DMA module to make it work in DMA mode.
 
2. In the terminal emulator running on PC, some information will be displayed to prompt
   users input 8 characters. 

3. The first descriptor for DMA to received 8 character will trigger the DMA interrupt A
   and the second descriptor for DMA to receive another 8 characters will trigger the DMA
   interrupt B. Project will print the received characters out and do a loop receive with
   case of the two descriptors link to each other.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso824MAX board
- Personal Computer

## Board Settings
------------------------
Connect SJ9: 2-3
## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J3 on the 
   LPCXpresso824MAX board).

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

5. Input 8 characters every time according to the prompt information displayed in terminal.
   Repeat the step 5.

## Expected Result
------------------------
Users are prompted to enter 8 characters according to the information displayed in the 
terminal emulator running on PC. Once the entered characters reach 8, board will echo back 
the received characters to the terminal emulator.  

***Note:*** Board will not immediately return each character to terminal emulator until
all 8 characters are all entered.
