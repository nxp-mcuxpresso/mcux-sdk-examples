Overview
========
The E-Compass demo application demonstrates the use of the FXOS8700 sensor. The tilt-compensated algorithm calculates
all three angles (pitch, roll, and yaw or compass heading).

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini USB cable
- FRDM-KV11Z board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
Note: you must rotate the board 360 degrees to get max , min value magnetic field.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
To calibrate Magnetometer, roll the board on all orientations to get max and min values
Press any key to start calibrating...

Calibrating magnetometer...
Calibrate magnetometer successfully!
Magnetometer offset Mx: 313 - My: 432 - Mz: 494
Compass Angle: xxxx
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
