Overview
========
This demo describes how to use SDK drivers to implement the PWM feature by FLEXIO IP module.
It outputs the PWM singal with fixed frequency defined by "DEMO_FLEXIO_FREQUENCY" in source code
and dynamic duty from 99 to 1 to one of the FLEXIO pin.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1020 board
- Personal Computer
- Oscilloscope

Board settings
==============
No special is needed.
Use oscilloscope to measure the output 48KHz PWM signal pin at J18-1.

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
When the example runs successfully, if the input duty cycle range is 0~100,
You can see similar information from the terminal as follows:
~~~~~~~~~~~~
FLEXIO_PWM demo start.

Please input a value (0 - 100) to set duty cycle: 10
Input value is 10

PWM duty cycle is: 10
PWM leave is: 0

Please input a value (0 - 100) to set duty cycle: 150
Input value is 150
Your value is output of range.
Set pwm output to IDLE.

Please input pwm idle status (0 or 1): 1
Input IDLE state value is 1

PWM leave is: 1 

Please input a value (0 - 100) to set duty cycle: 
......
~~~~~~~~~~~~
Note:
1. The duty cycle of the PWM is variable, except for the idle state, 0 and 100.
2. If the input duty cycle range exceeds 100, the pwm will be set to idle state
3. If the set pwm idle value is out of range, it will output "Your value is output of range."
   and then return to set the duty cycle.

Note:
To debug in qspiflash, following steps are needed:
1. Select the flash target and compile.
2. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J23.
3. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to qspiflash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
