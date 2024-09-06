Overview
========
The Power mode switch demo application demonstrates the use of power modes in the KSDK. The demo prints the power mode menu
through the debug console, where the user can set the MCU to a specific power mode. The user can also set the wakeup
source by following the debug console prompts. The purpose of this demo is to show how to switch between different power
 modes, and how to configure a wakeup source and wakeup the MCU from low power modes.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume addtional power, had better to disable related pins or disconnect them. 


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
JP7(2-3) connected. 
JP55(1-2) disconnected to ommit the impact of MCU-LINK during DPD and FDPD mode.

The board supports multiple power supply options. 
The code should be aligned with power supply on the board.

1. Default power supply, VDDN with PMIC, VDD1 and VDD2 with PMC.
   Jumper setting - JP1(Open), JP2(2-3), JP3(Open), JP4(1-2)
   Code setting(power_demo_config.h) - #define DEMO_POWER_SUPPLY_OPTION DEMO_POWER_SUPPLY_MIXED
2. Use PMIC to supply all. 
   Jumper setting - JP1(1-2), JP2(2-3), JP3(1-2), JP4(1-2)
   Code setting(power_demo_config.h) - #define DEMO_POWER_SUPPLY_OPTION DEMO_POWER_SUPPLY_PMIC

NOTE,
1. Please disconnect the debugger when measuring the power consumption.
2. For Full deep power down mode, please disconnect MCU-LINK and JP55 to ommit the impact of MCU-LINK circute.
3. To rebuild the HiFi cores' binary, use the main_dsp.c provided in the project root folder to replace the file in
   dsp_examples/dsp_naturedsp/<core_id> and build dsp_naturedsp examples to get the binaries for HiFi cores.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-LINK on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build cm33_core1 project first, then the cm33_core0 project.
4.  Download the core0 program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to
    begin running the demo.

Running the demo
================
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
For cm33_core0:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

####################  Power Mode Switch - CPU0 ####################

    Build Time: xxx  x 2024--16:45:55 
    Core Clock: 192134400Hz 
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. Deep Sleep Retention mode
	4. Deep power down mode
	5. Full deep power down mode
	6. Active test mode

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
For cm33_core1:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

####################  Power Mode Switch - CPU1 ####################

    Build Time: xxx  x 2024--16:44:27 
    Core Clock: 96038400Hz 
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. Deep Sleep Retention mode
	4. Deep power down mode
	5. Full deep power down mode
	6. Active test mode


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Notes,
1. When each of the CPU runs "6. Active test mode", the other core is expected to be in Deep sleep
or Deep Sleep Retention mode and should not change the power mode before the active test mode finished.
