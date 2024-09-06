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
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============
JP7(2-3) connected for 3.3V supply for VDDIO_2.
JP55(1-2) disconnected to ommit the impact of MCU-LINK during DPD and FDPD mode.

The board supports multiple power supply options. 
The code should be aligned with power supply on the board.

1. Default power supply, VDDN with PMIC, VDD1 and VDD2 with PMC.
   Jumper setting - JP1(Open), JP2(2-3), JP3(Open), JP4(1-2)
   Code setting(power_demo_config.h) - #define DEMO_POWER_SUPPLY_OPTION DEMO_POWER_SUPPLY_MIXED
2. Use PMIC to supply all. 
   Jumper setting - JP1(1-2), JP2(2-3), JP3(1-2), JP4(1-2)
   Code setting(power_demo_config.h) - #define DEMO_POWER_SUPPLY_OPTION DEMO_POWER_SUPPLY_PMIC

NOTE: When entering Deep Power Down or Full Deep Power Down modes, both core0 and core1 should 
request entering the mode. 

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-LINK USB port (J54) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the core0 program to the target board.
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
        Press C to enter  Deep Sleep Retention
        Press D to enter: Deep Power Down
        Press E to enter: Full Deep Power Down

Waiting for power mode select...

Next Loop

Please select the desired power mode:
        Press A to enter: Sleep
        Press B to enter: Deep Sleep
        Press C to enter  Deep Sleep Retention
        Press D to enter: Deep Power Down
        Press E to enter: Full Deep Power Down

Waiting for power mode select...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
