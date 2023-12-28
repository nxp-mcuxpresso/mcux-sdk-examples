Overview
========
The LED demo is used to demonstrate how to use new components (serial manager, LED, Button, GPIO adapter, Shell, Debug console, Uart adapter, Timer, and so on).
The main function of the demo is to control the led by using the shell or button.
For shell, please enter \"help\" to get the help information firstly. Turn on LED by using command \"led on\". And turn off LED by using command \"led off\".
If your board supports dimming, enhanch led brightness by using command \"dim brighten 5000\".And dim the brightness by using command \"dim darken 5000\".
For button, please press the button corresponding to control LED. Turn on LED when the button is pressed with long press or double click event.
And turn off LED when the button is pressed with short press or one click event.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini USB cable
- FRDM-K32L2A4S board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the Debug Link port on the board.
2. Download the program to the target board.
3. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the led control demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The demo is used to demonstrate how to use new components.
The main function of the demo is to control the led by using the shell or button.
For shell, please enter "help" to get the help information firstly. Turn on LED by using command "led on". And turn off LED by using command "led off".
For button, please press the button SW3 to control LED. Turn on LED when the button is pressed with long press or double click event. And turn off LED when the button is pressed with short press or one click event.

SHELL build: Aug 30 2018
 * Copyright 2019 NXP
led_control@SHELL>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

