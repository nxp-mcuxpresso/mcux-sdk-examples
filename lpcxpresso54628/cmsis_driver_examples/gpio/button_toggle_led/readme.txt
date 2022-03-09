Overview
========
The cmsis_gpio_button_toggle_led example show how to use the CMSIS driver interface to operate the LED as GPIO output and 
operate the GPIO as input with callback supported.

In this example, everytime button is pressed will trigger a interrupt and user callback will be invoked. The user callback will
toggle the LED and print to the console to indicate a button press event is detected.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CMSIS GPIO Example! 

Use Button to toggle LED! 

BUTTON Pressed! 

BUTTON Pressed! 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
