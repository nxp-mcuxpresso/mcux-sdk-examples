## Toolchain supported
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

## Board Settings
------------------------
Remove jumpers for JP7 and JP8, connect adjustable input voltage to JP8-2.

## Run the Demo
------------------------
1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the board).

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
The log below shows the output of the BOD demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOD INTERRUPT EXAMPLE.
Please adjust input voltage low than 2.24V to trigger BOD interrupt.

BOD interrupt occurred, input voltage is low than 2.24V.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
