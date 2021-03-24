Overview
========
The bubble application demonstrates basic usage of the on-board accelerometer to implement a
bubble level. A bubble level utilizes two axes to visually show deviation from a level plane
(0 degrees) on a given access. .


Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Oscilloscope
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the board is programmed, use oscilloscope probe to touch  
the R104/R107 arounding U9 module position, then simply tilt the board to see  the duty cycle change of the pwm.
One pwm indicates X-axis angle while another indicates Y-axis angle.
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~
Welcome to the BUBBLE example

You will see angle data change in the console when change the angles of board

x=     23 y =     56
x=      9 y =     53
x=     14 y =     49
......
~~~~~~~~~~~~
