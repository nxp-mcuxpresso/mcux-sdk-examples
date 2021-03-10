## Overview
-----------
This SCTIMer project is a demonstration program of the SDK SCTimer driver's PWM generation. It sets up a PWM signal
and periodically updates the PWM signals dutycycle. With an interrupt callback the PWM duty cycle is changed frequently 
in such a way that the LED brightness can be varied.

## Functional Description
-------------------------
In this example, The SCTimer module generates a PWM signal, dutycycle of which is changed constantly.

The example will initialize clock, pin mux configuration, , then configure the SCTimer module to generate 24KHZ PWM signal. 
Then, the example uses interrupt to update the PWM dutycycle on SCTimer output 4. Users can observe that LED brightness can be varied.

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
The log below shows example output of the SCTimer driver PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output center-aligned PWM signal

You will see a change in LED brightness if an LED is connected to the SCTimer output pin
If no LED is connected to the pin, then probe the signal using an oscilloscope
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Users can see that LED brightness can be varied on the board.
