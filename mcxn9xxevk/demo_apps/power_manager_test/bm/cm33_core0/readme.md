Overview
========
The power manager test application demonstrates the basic usage of power manager framework without RTOS.
The demo tests all features of power manager framework, including notification manager, wakeup source manager and so on.
The demo shows how the various power mode switch to each other based on power manager framework. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary module during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, the pin current would be activated unexpectedly waste some energy.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume additional power, had better to disable related pins or disconnect them.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Type-C USB cable
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============
Use the on-board MCULink and MCUXpresso IDE energy measurement function to measure the MCU current.
When use this method, please disconnect JP29.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J5.
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
The log below shows the output of the power manager test demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Power Manager Test.

Normal Boot.

Please select the desired power mode:
        Press A to enter: Sleep
        Press B to enter: Deep Sleep
        Press C to enter: Power Down
        Press D to enter: Deep Power Down

Waiting for power mode select...

Selected to enter Sleep.

 Select the wake up timeout in seconds.
 The allowed range is 1s - 9s.
 Eg. enter 5 to wake up in 5 seconds.
 Waiting for input timeout value...
 Will wakeup in 5 seconds.
 Enable VBAT.
 De-init UART.
 Re-init UART.
 Disable VBAT.
Next Loop

Please select the desired power mode:
        Press A to enter: Sleep
        Press B to enter: Deep Sleep
        Press C to enter: Power Down
        Press D to enter: Deep Power Down

Waiting for power mode select...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
