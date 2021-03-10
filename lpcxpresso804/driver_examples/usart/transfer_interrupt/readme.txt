## Overview
--------------------------------------------------------------------------------------
This example demonstrate configuration and use of the USART module in interrupt-driven 
asynchronous mode on communication with a terminal emulator calling the USART 
transactional APIs. USART will echo back every 8 characters.

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

## Functional description
---------------------------------------------------------------------------------------
In this example, USART communicates with a terminal emulator running on the PC, via 
the VCOM serial port. 

1. When users start running this project, main routine will initialize clock, pin mux 
   configuration, and configure the USART module to make it work in interrupt transactional
   mode.

2. In the terminal emulator running on PC, some information will be displayed to prompt
   users input 8 characters.
   
3. Once users entered 8 characters, main routine will echo back the received characters 
   to the terminal emulator and start for another loop transfer.
   
4. All read and write operations are executed in the interrupt handler(located in the 
   "fsl_usart.c" file) which is implemented in double weak mechanism, and the user does 
   not need to do other operations. But user can monitor the transmission state through
   the parameter `g_usartHandle`.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Mini/micro USB cable
- LPCXpresso804 board
- Personal Computer

##Board Settings
------------------------
No special settings are required.

## Run the Project
------------------------
Run the USART interrupt transfer example application by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the 
   LPCXpresso804 board).

2. Open a serial terminal emulator(for example PUTTY) in PC with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running the project.

5. Input several characters according to the prompt information displayed in terminal.
   Repeat the step 5.

## Expected Result
------------------------
Users are prompted to enter 8 characters according to the information displayed in the 
terminal emulator running on PC. Once the entered characters reach 8, board will echo back 
the received characters to the terminal emulator.
***Note:*** Board will not immediately return each character to terminal emulator until
all 8 characters are all entered
