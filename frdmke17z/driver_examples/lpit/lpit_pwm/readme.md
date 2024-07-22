Overview
========
This demo show how to use SDK drivers to implement the PWM feature by LPIT IP module in 
dual 16-bit Periodic counter mode,and clone by TRGMUX to output this PWM to TRGMUX_OUTx pin.
By default,the pwm duty is 50%, user can input from terminal to
change the period and duty cycle,use oscilloscope can measure this output PWM signal.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/Micro USB cable
- FRDM-KE17Z board
- Personal Computer

Board settings
==============
No special is needed.
Use oscilloscope to measure output the 1000Hz PWM signal pin at J1-5 pin of board,you can also
change pwm frequency and duty via terminal.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the PWM signal wave on oscilloscope and similar information from the terminal as below. 
~~~~~~~~~~~~~~~~~~~~~
lpit pwm demo start.

Please input pwm frequency and duty:
~~~~~~~~~~~~~~~~~~~~~
