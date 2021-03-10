## Overview
-----------
This example demonstrates configuration and use of the USART module in interrupt-driven 
asynchronous mode on communication with a terminal emulator calling the USART functional
APIs. USART will echo back every character to terminal emulator, and send back all 
received characters once users press `Enter`key.

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

1. Once the project start, main routine will initialize clock, pin mux configuration, 
   and configure the USART module to make it work in interrupt mode.
 
2. In the terminal on PC, user is prompted to input several characters and terminated 
   with `Enter` key. 

3. The USART IRQ handler will monitor the received data from the USART Rx(terminal) and 
   send back the same characters to terminal. IRQ Handler save the received characters 
   to the Rx buffer and check whether a `Enter` character is detected. If `Enter` is 
   detected, a flag `rxNewLineDetected` is set indicating a complete information is 
   received. 
   
4. The Main routine continuous checking the flag of `rxNewLineDetected` to decide whether
   to send the full string of the received inforamtion back to terminal. If yes, the 
   information will be sent back to terminal in interrupt-mode. USART IRQ handler will 
   check each IRQ triggered by USART transmit ready and fill the left data into the USART
   send register until all characters are sent. 

5. Main routinue repeats the check of the flag and send the received information to terminal. 


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
No special settings are required.

## Run the Project
------------------------
Run the USART terminal Example application by performing the following steps:

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

5. Input several characters followed with `Enter` key in terminal according to prompt
   information displayed in terminal. Repeat the step 5.

## Expected Result
------------------------
The input character will be direclty echoed back in the terminal. You will see what you have
typed in the terminal. When the `Enter` is input, the whole string will be echoed back to
the terminal.

***Note:*** The size of receive buffer is 32 bytes. Do not type characters more than 32 
beteween 2 `Enter`. If not, an notification of overflowed Rx buffer will be displayed in 
terminal and only the first 32 characters will be displayed.

