Overview
========
The power manager demo application demonstrates how to change power modes in the KSDK. The difference between this demo
and power_mode_switch is, this demo uses a notification framework to inform application about the mode change.
Application could register callback to the notification framework, when power mode changes, the callback
function is called and user can do something, such as closing debug console before entering low power mode, and
opening debug console after exiting low power mode.

When this demo runs, the power mode menu is shown in the debug console, where the user can set the MCU to a specific power mode.
User can also set the wakeup source following the debug console prompts.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXC444 board
- Personal Computer

Board settings
==============
No special board setting.

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
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
When running the demo, the debug console shows the menu to command the MCU to the target power mode.

NOTE: Only input when the demo asks for input. Input entered at any other time might cause the debug console to overflow
and receive the wrong input value.
~~~~~~~~~~~~~~~~~~~~~
####################  Power Manager Demo ####################

    Core Clock = 48000000Hz
    Power mode: RUN

Select the desired operation

Press  A for enter: RUN      - Normal RUN mode
Press  B for enter: WAIT     - Wait mode
Press  C for enter: STOP     - Stop mode
Press  D for enter: VLPR     - Very Low Power Run mode
Press  E for enter: VLPW     - Very Low Power Wait mode
Press  F for enter: VLPS     - Very Low Power Stop mode
Press  G for enter: LLS/LLS3 - Low Leakage Stop mode
Press  H for enter: VLLS0    - Very Low Leakage Stop 0 mode
Press  I for enter: VLLS1    - Very Low Leakage Stop 1 mode
Press  J for enter: VLLS3    - Very Low Leakage Stop 3 mode

Waiting for power mode select..
~~~~~~~~~~~~~~~~~~~~~
