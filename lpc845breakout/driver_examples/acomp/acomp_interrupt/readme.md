Overview
========

The ACOMP Interrupt Example shows how to use interrupt with ACOMP driver.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
ACOMP's negative channel input. On the postive side, the internal voltage ladder is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user-defined channel's voltage crosses the internal ladder's output. The change of
comparator's output would generate the falling and rising edge events with their interrupts enabled. When any ACOMP 
interrupt happens, the ACOMP's ISR would turn on/off the LED light.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

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
The log below shows the output of the ACOMP interrupt driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPC_ACOMP Interrupt Example.
The example compares analog input to the voltage ladder output(ACOMP negative port).
The LED will be turned ON/OFF when the analog input is LOWER/HIGHER than the ladder's output.
Change the analog input voltage to see the LED status.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
