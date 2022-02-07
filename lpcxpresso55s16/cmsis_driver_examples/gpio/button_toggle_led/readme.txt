Overview
========
The cmsis_gpio_button_toggle_led example show how to use the CMSIS driver interface to operate the LED as GPIO output and 
operate the GPIO as input with callback supported.

In this example, everytime button is pressed will trigger a interrupt and user callback will be invoked. The user callback will
toggle the LED and print to the console to indicate a button press event is detected.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- One LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J1) on the board.
2. Open a serial terminal on PC for JLink serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CMSIS GPIO Example! 

Use Button to toggle LED! 

BUTTON Pressed! 

BUTTON Pressed! 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
