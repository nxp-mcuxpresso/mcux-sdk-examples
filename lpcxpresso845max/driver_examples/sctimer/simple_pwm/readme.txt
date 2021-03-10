## Overview
-----------
The SCTimer project is a simple demonstration program of the SDK SCTimer's driver capabiltiy to generate PWM signals.
The PWM output can be observed with an oscilloscope.

## Functional Description
-------------------------
In this example, The SCTimer module generates two PWM signals.

The example will initialize clock, pin mux configuration, , then configure the SCTimer module to make it work. 
The example configures first 24kHZ PWM with 50% dutycycle from SCTIMER output 4. Then, The example configures second 24kHZ PWM with 20% 
negative duty cycle from SCTIMER output 2.

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
The log below shows example output of the SCTimer driver simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output 2 center-aligned PWM signals

Probe the signal using an oscilloscope
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect Pins J2-2(P0_28) and J2-3(P1_20) to an oscilloscope. Users can observe two 24kHZ PWM signals, one dutycycle is 50%,
the other one negative dutycycle is 20%.
