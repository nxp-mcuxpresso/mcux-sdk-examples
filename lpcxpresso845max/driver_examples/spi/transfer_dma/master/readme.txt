## Overview
-----------
This example demonstrates configuration and use of the SPI master module in dma-driven
mode on communication with a SPI slave module calling the SPI functional APIs. This example
should work together with the `spi_transfer_dma_slave` example.

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
In this example, SPI master will communicate with a slave module on another board, master 
will use dma mode and slave will use dma mode too.

1. Once the project starts, main routine will initialize clock, pin mux configuration, and
   configure the SPI module to make it work in master interrupt mode.
   
2. To make SPI master module run in dma mode, two dma channels should be configured for both 
   sending and receiving.

3. Before starting the SPI master transmission, descriptors should be created for dma 
   channels, it is easy to setup the receiving channel, only one descriptor is needed, but 
   for sending channel, an additional descriptor linking to the sending descriptor is needed
   to send the last frame data if users want to de-assert the SSEL pin after transfer is 
   completed.

4. Checking if the data from slave is all right when transfer complete, and print the 
   information to the debug console on PC.

5. De-initialize the SPI module.


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
Connect pins of SPI master and slave(on another board) as below:

    Master - SPI0                  Slave - SPI0   
Pin Name   Board Location      Pin Name   Board Location            
MISO       J2 pin 6            MISO       J2 pin 6                     
MOSI       J2 pin 7            MOSI       J2 pin 7                
SCK        J2 pin 5            SCK        J2 pin 5                 
SSEL0      J2 pin 8            SSEL0      J2 pin 8
GND        J1 pin 4            GND        J1 pin 4

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

4. Start the slave board first, then launch the debugger in your IDE to begin running this project.

5. Monitor the information on the debug console.

## Expected Result
------------------------
The SPI master module will communicate with slave module in dma mode, slave module 
should be started first. Once the communication between master board and slave board is 
completed, the received data and information will be printed to the debug console on PC.
