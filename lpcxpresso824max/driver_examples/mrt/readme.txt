## Overview
-----------
The MRT project is a simple demonstration program of the SDK MRT driver. It sets up the MRT
hardware block to trigger a periodic interrupt after every 250 milliseconds. When the MRT interrupt
is triggered a message to print on the UART terminal and the LED is toggled on the board.

## Functional Description
-------------------------
In this example, The MRT module enables interrupt to output message and toggle LED.

The example will initialize clock, pin mux configuration, initialize LED, then configure the MRT
module to make it work in Repeat Interrupt mode. When the interrupt occurs after every 250 milliseconds,
the pin output of LED is toggled and the terminal window print output.

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
No special settings are required.

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

5. Monitor the information on the debug console.

## Expected Result
------------------------
The log below shows example output of the MRT driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Starting channel No.0 ...
 Channel No.0 interrupt is occurred !
 Channel No.0 interrupt is occurred !
 Channel No.0 interrupt is occurred !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Users can see that LED is blinking.
