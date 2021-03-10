## Overview
-----------
This example demonstrates configuration and use of the I2C master module in dma-driven
mode on communication with a I2C slave module calling the I2C functional APIs. This example
should work together with the `lpc_i2c_dma_b2b_master` example.

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
  naming. If the code size and performance are critical requirements, see the transactional
  API implementation and write custom code.

## Functional Description
-------------------------
In this example, I2C slave will communicate with a master module on another board, master 
will use dma mode and slave will use interrupt mode.

1. Once the project starts, main routine will initialize clock, pin mux configuration, and
   configure the I2C module to make it work in slave interrupt mode.
   
2. Prepare the transfer of slave module by calling the transactional APIs, a handler will 
   be created to record the state of transfer, all read and write operations are executed 
   in the interrupt handler(located in the `fsl_i2c.c` file) which is implemented by 
   double weak mechanism. . 
   
3. Main routine will wait until the communication complete, checking if the data from 
   master is all right when transfer complete, and print the received data and information
   to the debug console on PC.


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
Connect pins of I2C master and slave(on another board) as below:

    Master - I2C0                  Slave - I2C0   
Pin Name   Board Location     Pin Name   Board Location
SCL        J1-1               SCL        J1-1
SDA        J1-2               SDA        J1-2
GND        J1-4               GND        J1-4

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J3 on the 
   LPCXpresso824MAX board) for receiving debug information.

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running this project, start the master example 
   on another board.

5. Monitor the information on the debug console.

## Expected Result
------------------------
The I2C slave module will communicate with master module in interrupt mode, slave module 
should be started first. Once the communication between master board and slave board is 
completed, the received data and information will be printed to the debug console on PC.
