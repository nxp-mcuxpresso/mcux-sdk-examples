## Overview
-----------
This example demonstrates configuration and use of the USART module in polling-driven 
asynchronous mode on communication with a terminal emulator calling the USART functional
APIs. USART will echo back every 8 characters to terminal emulator.

**Transactional API and Functional API**

- Low level functional APIs provide common peripheral functionality, abstracting the 
  hardware peripheral register access into a set of state less basic functional operations.
  These APIs primarily focus on the control, configuration, and function of basic 
  peripheral operations. 

- Transactional APIs provide a quick method for customers to utilize higher-level 
  functionality of the peripherals. The transactional APIs utilize interrupts and perform 
  asynchronous operations without user intervention. Transactional APIs operate on high-
  level logic that requires data storage for internal operation context handling. However,
  the peripheral drivers do not allocate this memory space. Rather, the user passes in the
  memory to the driver for internal driver operation. 

- All driver examples calling the transactional API get the `transfer` in the project 
  naming. If the code size and performance are critical requirements, see the 
  transactional API implementation and write custom code.

## Functional Description
-------------------------
In this example, USART communicates with a terminal emulator running on the PC, via 
the VCOM serial port. 

When start running this example, main routine will initialize clock, pin mux configuration,
and configure the USART module to make it work in polling way. Users are prompted to 
input several characters, board will echo back the received characters to the terminal
immediately. 

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
- Personal Computer

## Board Settings
------------------------
No special settings are required.

## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the 
   LPCXpresso804 board).

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
Users are prompted to input several characters at the terminal emulator on PC, the input
character will be echoed back in the terminal immediately. Users will see what they have 
typed in the terminal. 
