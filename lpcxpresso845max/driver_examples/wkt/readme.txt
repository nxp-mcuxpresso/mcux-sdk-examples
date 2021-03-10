## Overview
-----------
The WKT project is a simple demonstration program of the SDK WKT driver. It sets up the WKT
hardware block to trigger a periodic interrupt after loading a counter value and counting down to 0. 
When the WKT interrupt is triggered a message printed on the UART terminal and the LED is toggled on the board.

Depending on the clock source, the WKT can be used for waking up the part from any low power mode or for 
general-purpose timing.

## Functional Description
-------------------------
In this example, The WKT module enables interrupt to output message and toggles LED.

The example will initialize clock, pin mux configuration, initialize LED, then configure the WKT module to make 
it work. The example sets counter value to start the timer counting. Timer counts down to 0, then stop and generates 
an interrupt. The terminal window print output and the pin output of LED is toggled. When a new count value is loaded, 
timer starts again.

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
Run the example by performing the following steps:

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

5. Monitor the information on the debug console.

## Expected Result
------------------------
The log below shows example output of the WKT driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WKT interrupt example

 Self-wake-up timer interrupt is occurred !
 Self-wake-up timer interrupt is occurred !
 Self-wake-up timer interrupt is occurred !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Users can see that LED is blinking.
