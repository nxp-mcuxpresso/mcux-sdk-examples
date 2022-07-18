Overview
========
The SCTImer multi-state project is a demonstration program of the SCTimer state machine. It shows how to set up events to be triggered in a certain state
and transitioning between states.
State 0 has 2 events that generate a PWM signal, it also has an event linked to an input signal to transition to State 1.
State 1 has 4 events that generate 2 PWM signals, it also has an event linked to an input signal to transition to State 0.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer

Board settings
==============
No special settings are required.

1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the SCTimer driver simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output edge-aligned PWM signal

When user presses a switch the PWM signal will be seen from Out 3
When user presses the switch again PWM signal on Out 3 will turn off
The PWM signal from Out 0 will remain active all the time
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use Oscilloscope to measure and observe the TP6(out 0) and TP7(out 3) output signal.
