Overview
========
This demo application demonstrates the EflexPWM fault demo.
This application demonstrates the pulse with modulation function of EflexPWM module. It outputs the
PWM to control the intensity of the LED. PWM shut down when a fault signal is detected on the CMP
output. One input of CMP, other input is from internal DAC.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1060 board
- Personal Computer
- Oscilloscope

Board settings
==============
Weld 0Ω resistor to R280.

* Probe the pwm signal using an oscilloscope
 - At J24-6

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
The log below shows the output of the pwm fault demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Welcome to PWM Fault demo
Use oscilloscope to see PWM signal at probe pin: J24-6
Connect pin J23-5 to high level and ground to see change.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The level state on pin J23-5 determines the output of the PWM signal
- When at low level, the PWM signal will output at pin J24-6.
- When at high level, the PWM signal output will be disabled.
