## Overview
-----------
This example demonstrates configuration and use of the SPI slave module in interrupt-driven
mode on communication with a SPI master module calling the SPI transactional APIs. This 
project should work together with `spi_transfer_interrupt_master` example.

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

**Double weak mechanism used for transactional APIs **

- A double weak mechanism is introduced for drivers with transactional API. The double 
  weak indicates two levels of weak vector entries. See the examples below: 

```assembly
        PUBWEAK SPI0_IRQHandler
        PUBWEAK SPI0_DriverIRQHandler
SPI0_IRQHandler
        LDR     R0, =SPI0_DriverIRQHandler
        BX      R0
```

- The first level of the weak implementation are the functions defined in the vector 
  table. The drivers with transactional APIs provide the reimplementation of the second
  layer function inside of the peripheral driver. If the drivers with transactional APIs 
  are linked into the image, the `SPI0_DriverIRQHandler` is replaced with the function
  implemented in the SPI driver. 

- The reason for implementing the double weak functions is to provide a better user 
  experience when using the transactional APIs. For drivers with a transactional function,
  call the transactional APIs and the drivers complete the interrupt-driven flow. Users are
  not required to redefine the vector entries out of the box. At the same time, if users 
  are not satisfied by the second layer weak function implemented in the drivers, users 
  can redefine the first layer weak function and implement their own interrupt handler 
  functions to suit their implementation.
  
## Functional Description
-------------------------
In this example, SPI slave will communicate with a master module on another board, 
master will use polling mode and slave will use interrupt transactional mode.

1. Once the project starts, main routine will initialize clock, pin mux configuration, 
   and configure the SPI module to make it work in slave interrupt mode to communicate
   with master module.

2. Prepare the transfer of slave module by calling the transactional APIs, a handler will 
   be created to record the state of transfer, all read and write operations are executed 
   in the interrupt handler(located in the `fsl_spi.c` file) which is implemented by 
   double weak mechanism.  

3. Main routine will wait until the communication complete, checking if the data from 
   master is all right when transfer complete, and print the received data and information
   to the debug console on PC.

4. De-initialize the SPI module.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso802 board
- Personal Computer

## Board Settings
------------------------
Connect pins of SPI master and slave(on another board) as below:

    Master - SPI0                  Slave - SPI0   
Pin Name   Board Location      Pin Name   Board Location            
MISO       CN3 pin 19          MISO       CN3 pin 19
MOSI       CN3 pin 21          MOSI       CN3 pin 21
SCK        CN3 pin 15          SCK        CN3 pin 15                
SSEL0      CN3 pin 22          SSEL0      CN3 pin 22

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN1 on the 
   LPCXpresso802 board) for receiving debug information.

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
The SPI slave module will communicate with slave module in interrupt mode, slave module 
should be started first. Once the communication between master board and slave board is 
completed, the received data and information will be printed to the debug console on PC.
