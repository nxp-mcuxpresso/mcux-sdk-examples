Overview
========
The power_mode_switch_lpc application shows the usage of normal power mode control APIs for entering the four kinds of
low power mode: Sleep mode, Deep Sleep mode and Power Down mode, deep power down mode. When the application runs to each low power
mode, the device would cut off the power for specific modules to save energy. The device can be also waken up by
prepared wakeup source from external event.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary module during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, the pin current would be activated unexpectedly waste some energy.
 - Other low power consideration based on the actual application hardware.
 - In order to meet typedef power consumption of DateSheet manual, Please configure MCU under the following conditions.
     • Configure all pins as GPIO with pull-up resistor disabled in the IOCON block.
     • Configure GPIO pins as outputs using the GPIO DIR register.
     • Write 1 to the GPIO CLR register to drive the outputs LOW.
     • All peripherals disabled.

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
- LPCXpresso860MAX board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J4) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
## Expected Result
------------------------
Power mode switch Demo for LPC8xx.

Selecct an option

    1. Sleep mode

    2. Deep Sleep mode

    3. Power Down mode

    4. Deep power down mode
/* after select power mode, terminal will output */
Select wakeup source

    1. Wkt timer

    2. SW2, wakeup key

    3. SW3, reset key
/* after wakeup, terminal will output */
Wakeup.
