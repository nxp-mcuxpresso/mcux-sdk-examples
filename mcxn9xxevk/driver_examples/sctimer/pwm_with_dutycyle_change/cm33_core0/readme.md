Overview
========
This SCTIMer project is a demonstration program of the SDK SCTimer driver's PWM generation. It sets up a PWM signal
and periodically updates the PWM signals dutycycle.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N9XX-EVK board
- Personal Computer

# Board settings

```
Output signal		Board location
SCT0_OUT4    		J3-3 (P1_22)
```

# Prepare the Demo

1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the SCTimer driver PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output center-aligned PWM signal
You will see a change in LED brightness if an LED is connected to the SCTimer output pin
If no LED is connected to the pin, then probe the signal using an oscilloscope
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You'll see  PWM signals with changing duty cycle on J3_3 using an oscilloscope. 
