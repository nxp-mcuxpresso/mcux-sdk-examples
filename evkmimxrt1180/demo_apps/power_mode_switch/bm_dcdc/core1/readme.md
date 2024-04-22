Overview
========
The Power mode switch demo application demonstrates the usage of power modes. 
The CPU mode is power mode of CPU platform. Each CPU platform has its own power mode. They are RUN, WAIT, STOP, SUSPEND.
Some resource that are managed at system level, and are not owned and controlled by any of the CPU platform are called as public resources.
Set point is used to control the power state of the public resources. With CPU Mode and Set Point defined properly, 
the Power Mode of a system is defined as a combination of the CPU Mode and Set Point.
The Standby mode is the third kind of low power mode besides CPU mode and set point,
it is related to state of all CPU platform and has a much shorter transition time than setpoint.
Only when all CPU platforms send standby request can the system can enter into standby mode.

This demo prints the power mode menu through the debug console, where the user can set the MCU to a specific power mode.
User can wakeup the core by key interrupt. The purpose of this demo is to show how to switch between different power modes,
and how to configure a wakeup source and wakeup the MCU from low power modes.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary module during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, the pin current would be activated unexpectedly waste some energy.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume addtional power, had better to disable related pins or disconnect them. 


SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
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
1. When running the demo, the debug console shows the menu to provide the desired operation. Here we take multicore targets as an example.

This is core0.
Copy Secondary core image to address: 0x303c0000, size: XXXXX
Starting Secondary core.
The secondary core application has been started.

CPU wakeup source 0x10001...

***********************************************************
	Power Mode Switch Demo for iMXRT1189
***********************************************************

Core0 send message to core1.

M33 previous CPU mode is RUN
M33 current CPU mode is RUN
M33 CLK is 240 MHz
M7 previous CPU mode is RUN
M7 current CPU mode is RUN
M7 CLK is 798 MHz
EDGELOCK CLK is 200 MHz
BUS AON CLK is 132 MHz
BUS WAKEUP CLK is 264 MHz
WAKEUP AXI CLK is 240 MHz


Please select the desired operation:
Press  A to demonstrate run mode switch.
Press  B to demonstrate cpu mode switch.
Press  C to enter BBSM mode.

Waiting for select...

2. When 'A' is selected, the menu shows selections for different run mode.

RUN mode switch:
Press A to enter OverDrive RUN
Press B to enter Normal RUN
Press C to enter UnderDrive RUN
Press 'Q' to exit

Waiting for select...

3. When 'B' is selected, the menu shows selections for different CPU mode with/without system sleep request.

CPU mode switch:
Press A to enter CPU mode: RUN
Press B to enter CPU mode: WAIT
Press C to enter CPU mode: STOP
Press D to enter CPU mode: SUSPEND
Press E to enter CPU mode: STOP, system sleep
Press F to enter CPU mode: SUSPEND, system sleep
Press 'Q' to exit

Waiting for select...

3. When 'C' is selected, the menu shows wakeup source selection for BBSM mode.
System will wake up from reset when wakeup event happening.

Select the wake up source:
Press T for Timer
Press S for GPIO button SW8. 

Waiting for key press..

Note
Reset event will happen when running this demo, so container header is needed. Please add container header by spsdk.
