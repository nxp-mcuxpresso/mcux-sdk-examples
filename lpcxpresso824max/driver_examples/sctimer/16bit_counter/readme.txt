## Overview
-----------
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters. The example toggles an output per counter when a match occurs.

## Functional Description
-------------------------
In this example, The SCTimer module generates two PWM signals.

The example will initialize clock, pin mux configuration, , then configure the SCTimer module to make it work. 
The example schedules a match event for Counter L every 0.1 seconds and schedules a match event for Counter H every 0.2 seconds. SCTimer output 2 is toggled when Counter L event occurs. SCTimer output 4 is toggled when Counter H event occurs.

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
The log below shows example output of the SCTimer 16-bit counter demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to use it in 16-bit mode

The example shows both 16-bit counters running and toggling an output periodically
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect Pins J2-3(P0_28) and J2-2(P0_16) to an oscilloscope. Users can observe two PWM signals.
