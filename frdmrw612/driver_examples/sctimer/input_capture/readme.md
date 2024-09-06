Overview
========
The SCTimer project is a demonstration program of the SDK SCTimer driver's input capture feature.
This example uses Event 0 to 4, State 0 and 1, Capture 0 to 3, Match 4, one user defined input.

If high level input is detected on the channel when SCTimer timer starts, SCTimer will generate a
capture event incorrectly although there is no rising edge. So this example ignore first pluse and
capture second pluse.
Event 0 and 1 occur in state 0. They are triggered by first rising and falling edge. After event 1
is triggered, new state is state 1. Event 2 and 3 are triggered continuously by following rising
and falling edge in state 1. Event 0,1,2,3 cause Capture 0,1,2,3 individually. SCTimer capture second
pluse in state 1.
Event 4 is used as counter limit(reset). Event 4 causes Match 4, match value is 65535.

This example gets the capture value of the input signal using API SCTIMER_GetCaptureValue() in Event
2 and 3 interrupt.

Need to ensure to input least two pluse into the channel, a pwm signal is recommended.
This example will print the capture values and pluse width of the input signal on the terminal window.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
Connect J1-1(SCT In 1) to 1kHz, dutycycle 50% PWM signal.

1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board
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
SCTimer edge capture example

Once the input signal is received the capture values are printed
The input signal's pulse width is calculated from the capture values & printed

Capture2: 12301 Capture3: 17265 Pluse Width: 500 us
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
