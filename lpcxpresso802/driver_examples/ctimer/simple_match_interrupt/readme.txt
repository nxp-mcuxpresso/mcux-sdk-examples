## Overview
-----------
The Simple Match Interrupt project is to demonstrate usage of the SDK CTimer driver with interrupt callback functions.
In this example the upon match and IO pin is toggled and the timer is reset, so it would generate a square wave.
With an interrupt callback the match value is changed frequently in such a way that the frequency of the output square wave is increased gradually.

## Functional Description
-------------------------
In this example, The CTimer module generates two PWM signals.

When start running this example, main routine will initialize clock, pin mux configuration, then configure the CTIMER module and enable interrupt to generate PWM signals. When a match occurs, CTimer will toggle the output level. The match value is changed by changed by interrupt callback. 
Users can see that the frequency of PWM signial is increased gradually by oscilloscope.

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
The log below shows example output of the CTimer simple match demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer match example to toggle the output.
This example uses interrupt to change the match period.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect Pins CN3-28(P0_9) and CN3-25(P0_8) to an oscilloscope. Users can observe two PWM signals, the frequency of which is increased gradually.
