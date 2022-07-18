Overview
========
The Simple Match Interrupt project is to demonstrate usage of the SDK CTimer driver with interrupt callback functions
In this example the upon match and IO pin connected to the LED is toggled and the timer is reset, so it would generate a square wave.
With an interrupt callback the match value is changed frequently in such a way that the frequency of the output square wave is increased gradually.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J1) on the target board.
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
The log below shows example output of the CTimer simple match demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer match example to toggle the output. 
This example uses interrupt to change the match period.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can see the red led and green led will blinking alternating frequently and then return to the original status.  
