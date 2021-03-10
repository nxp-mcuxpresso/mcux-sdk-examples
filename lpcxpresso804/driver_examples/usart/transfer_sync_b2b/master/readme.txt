## Overview
-----------
This example demonstrates configuration and use of the USART module in synchronous mode.
Master will send data to slave on one board, 

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
        PUBWEAK USART0_IRQHandler
        PUBWEAK USART0_DriverIRQHandler
USART0_IRQHandler
        LDR     R0, =USART0_DriverIRQHandler
        BX      R0
```

- The first level of the weak implementation are the functions defined in the vector 
  table. The drivers with transactional APIs provide the reimplementation of the second
  layer function inside of the peripheral driver. If the drivers with transactional APIs 
  are linked into the image, the `USART0_DriverIRQHandler` is replaced with the function
  implemented in the  USART driver. 

- The reason for implementing the double weak functions is to provide a better user 
  experience when using the transactional APIs. For drivers with a transactional function,
  call the transactional APIs and the drivers complete the interrupt-driven flow. Users are
  not required to redefine the vector entries out of the box. At the same time, if users 
  are not satisfied by the second layer weak function implemented in the drivers, users 
  can redefine the first layer weak function and implement their own interrupt handler 
  functions to suit their implementation.

## Functional Description
-------------------------
In this example, USART will send data to slave on one board, the project inforamtion will
be displayed in the terminal emulator running on PC.

1. Once the project start, main routine will initialize clock, pin mux configuration, 
   and configure one USART module as master and another USART module as slave.
 
2. A transactional API will be used for slave instance to receive data from master instance
   which is in polling-driven mode, once the transmission is complete, the inforamtion will 
   be print out to terminal emulator telling users if this transmission is success.

3. The slave is in interrupt-driven mode, all read and write operations are executed in the 
   interrupt handler(located in the "fsl_usart.c" file) which is implemented in double weak
   mechanism, and the user does not need to do other operations. 
   
4. De-initialize the USART instances after the transmission is complete.

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
Please connect the USART1_CLK pin to USART1_CLK pin directly.
No need to connect other pins, because the data pins of USART master and slave are multiple
used. Just connect the pins like below:

Master - USART1                Slave - USART1   
Pin Name   Board Location      Pin Name   Board Location            
TXD       CN3 pin 10           RXD       CN3 pin 9                   
RXD       CN3 pin 9            TXD       CN3 pin 10               
SCK       CN8 pin 2            SCK       CN8 pin 2               


## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the 
   LPCXpresso802 board), and make sure the board settings has been set up correctly.

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

5. Monitor the inforamtion displayed in the terminal emulator to check if the transmission 
   is success.

## Expected Result
------------------------
The debug console will print some information to the terminal emulator running on PC
to tell users if the transmission is success after the transmission between master and
slave is complete.

