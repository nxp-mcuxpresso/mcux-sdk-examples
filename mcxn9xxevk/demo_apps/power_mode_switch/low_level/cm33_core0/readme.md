Overview
========
The power_mode_switch_ll demo application demonstrates the usage of low level power-related drivers(SPC, CMC, VBAT, WUU, and so on) to enter/exit different power modes.
    - WUU: Help pins and modules generate a temporary wake-up from Power Down or Deep Power Down mode.
    - CMC: Provide the sequencing of the CPU and associated logic through the different modes.
    - SPC: System power controller which is used to configure on-chip regulators in active/low-power modes.

The demo prints the power mode menu through the debug console, where the users can set the MCU to a specific power modes. Users can also select the wake-up source by following the debug console prompts. The purpose of this demo is to show how to switch between different power modes, and how to configure a wake-up source and wakeup the MCU from low power modes.
This demo demonstrates 4 level power modes:
    - Sleep: CPU0 clock is off, and the system clock and bus clock remain ON. Most modules can remain operational.
    - Deep Sleep: Core clock, system clock, and bus clock are all OFF, some modules can remain operational with low power asynchronous clock source(OSC32k or FRO16K which located in VBAT domain) and serve as wake-up sources.
    - Power Down:  Core, system, bus clock are gated off, the VDD_CORE_MAIN and VDD_CORE_WAKE power domains are put into retention mode which means modules in these domains are not operational.
    - Deep Power Down: The whole VDD_CORE domain is power gated. Both CORE_LDO and DCDC are turned off. The device wakes from Deep Power Down mode through the Reset routine.
This demo demonstrates 2 wake-up sources:
    - LPTMR: Located in System power domain, clocked by FRO16K which from VBAT domain. LPTMR is also enabled in WUU domain to wake-up device from power down and deep power down modes.
    - WakeUpButton: The external pin which connected to WUU.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MCXN9XX-EVK Board
- Personal Computer

Board settings
==============
1. To wakeup the chip via SW4, please populate R198.
2. To measure current consumption via MCULink, please remove JP29.

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

NOTE: 
1.Only input when the demo asks for input. Input entered at any other time might cause the debug console to overflow
and receive the wrong input value.
2. Wake-up from Deep Power Down mode through wakeup reset that will reset the program("Normal Boot" will be printed).
3. The signal output from J2_7 can be used to observe entry/exit of DeepSleep, PowerDown, and Deep Power Down mode.
~~~~~~~~~~~~~~~~~~~~~
Normal Boot.

###########################    Power Mode Switch Demo    ###########################
    Core Clock = 48000000Hz
    Power mode: Active

Select the desired operation

        Press A to enter: Active mode
        Press B to enter: Sleep mode
        Press C to enter: DeepSleep mode
        Press D to enter: PowerDown mode
        Press E to enter: DeepPowerDown mode

Waiting for power mode select...

~~~~~~~~~~~~~~~~~~~~~
