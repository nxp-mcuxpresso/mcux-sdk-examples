## Overview
-----------
The CTimer Example project is to demonstrate usage of the KSDK ctimer driver. In this example, CTimer is used to generate a PWM signal. The PWM output can be observed with an oscilloscope.

## Functional Description
-------------------------
In this example, The CTimer module generates a PWM signal.

When start running this example, main routine will initialize clock, pin mux configuration, and configure the CTIMER module to generate a 20Khz PWM signal with 20% dutycycle.

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
No special settings are required.

## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN1 on the 
   LPCXpresso802 board).

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
The log below shows example output of the CTimer simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer example to generate a PWM signal
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect Pin P0_9(CN3-28) to an oscilloscope, Users can observe a 20Khz PWM signal with 20% dutycycle.
