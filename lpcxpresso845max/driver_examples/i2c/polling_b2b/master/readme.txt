## Overview
-----------
This example demonstrates configuration and use of the I2C master module in polling mode 
on communication with a I2C slave module calling the I2C functional APIs. This example
should work together with `lpc_i2c_polling_b2b_transfer_slave` example.

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
In this example, I2C master will communicate with a slave module on another board, master 
will use polling mode and slave use interrupt transactional mode.

1. Once the project starts, main routine will initialize clock, pin mux configuration, 
   and configure the I2C module to make it work in master mode.

2. Prepare transfer for master to communicate with slave module calling the blocking API, 
   In this API, main routine will check the status of register before sending and receiving
   data, and this API will not return until all data is sent and received.
   
3. Checking if the data from slave module is all right while transfer is completed, and print
   the received data and information to the debug console on PC.

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
Connect pins of I2C master and slave(on another board) as below:

    Master - I2C0                  Slave - I2C0   
Pin Name   Board Location     Pin Name   Board Location
SCL        J1-1               SCL        J1-1
SDA        J1-2               SDA        J1-2
GND        J1-4               GND        J1-4

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the 
   LPCXpresso845MAX board) for receiving debug information.

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
The I2C master module will communicate with slave module in polling mode, slave module 
should be started first. Once the communication between master board and slave board is 
completed, the received data and information will be printed to the debug console on PC.
