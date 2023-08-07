Overview
========

The ACOMP Basic Example shows the simplest way to use ACOMP driver and help user with a quick start.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
ACOMP's negative channel input. On the postive side, the internal voltage ladder is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user's voltage crosses the internal ladder's output. The endless loop in main() function
would detect the logic value of comparator's output, and change the LED. The LED would be turned on when the compare
output is logic one, or turned off when zero.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPC845 Breakout board
- Personal Computer

Board settings
==============
PIO0_14 is ACOMP negative input pin, which can sample external voltage.

Prepare the demo
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the ACOMP basic driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPC_ACOMP Basic Example.
The example compares analog input to the voltage ladder output(ACOMP negative port).
The LED will be turned ON/OFF when the analog input is LOWER/HIGHER than the ladder's output.
Change the analog input voltage to see the LED status.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
