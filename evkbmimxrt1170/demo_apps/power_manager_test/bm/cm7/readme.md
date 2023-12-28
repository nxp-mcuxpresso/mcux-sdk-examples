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
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- USB to Com Converter

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
When running the demo, the debug console shows the menu to provide the desired operation.

Power Manager Test.

Normal Boot.
Previous setpoint is 0.
Current setpoint is 0.

Please select the desired power mode:
        Press A to enter: Setpoint0, CM7 domain WAIT
        Press B to enter: Setpoint1, CM7 domain STOP
        Press C to enter: Setpoint2, CM7 domain STOP
        Press D to enter: Setpoint3, CM7 domain STOP
        Press E to enter: Setpoint4, CM7 domain STOP
        Press F to enter: Setpoint5, CM7 domain STOP
        Press G to enter: Setpoint6, CM7 domain STOP
        Press H to enter: Setpoint7, CM7 domain STOP
        Press I to enter: Setpoint8, CM7 domain STOP
        Press J to enter: Setpoint9, CM7 domain SUSPEND
        Press K to enter: Setpoint10, CM7 domain SUSPEND

Waiting for power mode select...


Note:
To debug in external flash, following steps are needed:
1. Select the flash target and compile.
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to external flash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
   
Note:
Flexspi_nor_debug and Flexspi_nor_release targets on armgcc toolchain need to click SW4(reset button) to run cases after downloading the image.
