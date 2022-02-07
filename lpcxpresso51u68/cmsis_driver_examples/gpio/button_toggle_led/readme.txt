Overview
========
The cmsis_gpio_button_toggle_led example show how to use the CMSIS driver interface to operate the LED as GPIO output and 
operate the GPIO as input with callback supported.

In this example, everytime button is pressed will trigger a interrupt and user callback will be invoked. The user callback will
toggle the LED and print to the console to indicate a button press event is detected.

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso51U68 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J6) on the board
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CMSIS GPIO Example! 

Use Button to toggle LED! 

BUTTON Pressed! 

BUTTON Pressed! 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
