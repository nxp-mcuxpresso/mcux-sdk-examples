Overview
========
The Power mode switch demo application demonstrates the use of power modes in the KSDK. The demo prints the power mode menu
through the debug console, where the user can set the MCU to a specific power mode. The user can also set the wakeup
source by following the debug console prompts. The purpose of this demo is to show how to switch between different power
 modes, and how to configure a wakeup source and wakeup the MCU from low power modes.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary module during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, the pin current would be activated unexpectedly waste some energy.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume addtional power, had better to disable related pins or disconnect them. 


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============
1. To wakeup the chip via VBAT, please connect a signal to TP1, and the falling edge will wakeup the chip.
2. To test PowerSwitchOff mode, please remove jumper for JP6(1-2) and short JP6(2-3), but to test other modes
   please short JP6(1-2).
3. The data of power consumption is measured on KW45B41Z-EVK RevD JP5(3-4) which is in DCDC mode. 

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
When running the demo, the debug console shows the menu to command the MCU to the target power mode.

NOTE: Only input when the demo asks for input. Input entered at any other time might cause the debug console to overflow
and receive the wrong input value.
~~~~~~~~~~~~~~~~~~~~~
###########################    Power Mode Switch Demo    ###########################
    Core Clock = 96000000Hz
    Power mode: Active

Please Select the desired power mode:

        Press A to enter Active mode!
        Press B to enter Sleep1 mode!
        Press C to enter DeepSleep1 mode!
        Press D to enter DeepSleep2 mode!
        Press E to enter DeepSleep3 mode!
        Press F to enter DeepSleep4 mode!
        Press G to enter PowerDown1 mode!
        Press H to enter PowerDown2 mode!
        Press I to enter PowerDown3 mode!
        Press J to enter PowerDown4 mode!
        Press K to enter DeepPowerDown1 mode!
        Press L to enter DeepPowerDown2 mode!
        Press M to enter PowerSwitchOff mode!

        Waiting for power mode select...


~~~~~~~~~~~~~~~~~~~~~
