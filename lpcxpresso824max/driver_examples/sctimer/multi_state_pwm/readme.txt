## Overview
-----------
The SCTImer multi-state project is a demonstration program of the SCTimer state machine. It shows how to set up events to be triggered 
in a certain state and transitioning between states.
State 0 has 2 events that generate a PWM signal, it also has an event linked to an input signal to transition to State 1.
State 1 has 4 events that generate 2 PWM signals, it also has an event linked to an input signal to transition to State 0.

## Functional Description
-------------------------
In this example, The SCTimer module sets up events to generate PWM signals.

1. The example will initialize clock, pin mux configuration, , then configure the SCTimer module to make it work.
2. The example configures PWM params and schedules events for generating a 24KHz PWM with 10% duty cycle from Out 4 in State 0, schedules an event to look for a rising edge on input 1 in State 0, then transits to next State when a rising edge is detected on input 1.
3. The example go to State 1, schedules events for generating a 24KHz PWM with 50% duty cycle from Out 2 in State 1, schedules the period event and the pulse event for the PWM, then schedules an event to look for a rising edge on input 1 in State 1.
4. The example transits back to State 0 when a rising edge is detected on input 1, start the timer to use counter L.

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
The log below shows example output of the SCTimer multi-state demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output edge-aligned PWM signal

When user presses a switch the PWM signal will be seen from Out 2
When user presses the switch again PWM signal on Out 2 will turn off
The PWM signal from Out 4 will remain active all the time
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect Pins J2-3(P0_28) and J2-2(P0_16) to an oscilloscope. Users can press the SW2 to switch states.
In State 0, Users can observe a PWM signal. In State 1, Users can observe two PWM signals.
