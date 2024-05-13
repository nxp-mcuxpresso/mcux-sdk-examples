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
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
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
Copy Secondary core image to address: 0x20200000, size: XXXXX
Starting secondary core.
The secondary core application has been started.

CPU wakeup source 0x10001...

***********************************************************
	Power Mode Switch Demo for iMXRT1176
***********************************************************

Core0 send message to core1.

System previous setpoint is 0
System current setpoint is 0
M7 previous CPU mode is RUN
M7 current CPU mode is RUN
M7 CLK is 696 MHz
M4 previous CPU mode is RUN
M4 current CPU mode is RUN
M4 CLK is 240 MHz

Please select the desired operation:
Press  A to demonstrate typical set point transition.
Press  B to demonstrate cpu mode switch in setpoint 0.
Press  C to enter SNVS mode.

Waiting for select...

2. When 'A' is selected, the menu shows selections for setpoint and CPU mode combination.
If CPU enters SUSPEND mode, system will wake up from reset when wakeup event happening.
Note: When user select F or G, CPU will be forced to enter SUSPEND mode since power domain WAKEUPMIX is off at setpoint 11~15. System will wake up from reset when wakeup event happening.

Set Point Transition:
Press A to enter Set Point: 1
    M7 CPU mode: RUN
    M4 CPU mode: RUN
Press B to enter Set Point: 0
    M7 CPU mode: RUN
    M4 CPU mode: RUN
Press C to enter Set Point: 5
    M7 CPU mode: RUN
    M4 CPU mode: RUN
Press D to enter Set Point: 7
    M7 CPU mode: RUN
    M4 CPU mode: STOP
Press E to enter Set Point: 9
    M7 CPU mode: STOP
    M4 CPU mode: RUN
Press F to enter Set Point: 11
    M7 CPU mode: SUSPEND
    M4 CPU mode: RUN
Press G to enter Set Point: 12
    M7 CPU mode: SUSPEND
    M4 CPU mode: RUN
Press H to enter Set Point: 1
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press I to enter Set Point: 0
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press J to enter Set Point: 5
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press K to enter Set Point: 7
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press L to enter Set Point: 10
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press M to enter Set Point: 11
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press N to enter Set Point: 15
    M7 CPU mode: SUSPEND
    M4 CPU mode: SUSPEND
    System standby
Press 'Q' to exit

Waiting for select...

3. When 'B' is selected, the menu shows selections for different CPU mode with/without system standby request in setpoint0.
If CPU enters SUSPEND mode, system will wake up from reset when wakeup event happening.

Cpu mode switch:
Press A to enter cpu mode RUN
Press B to enter cpu mode WAIT
Press C to enter cpu mode STOP
Press D to enter cpu mode SUSPEND
Press E to enter cpu mode WAIT, system standby
Press F to enter cpu mode STOP, system standby
Press G to enter cpu mode SUSPEND, system standby
Press 'Q' to exit

Waiting for select...

3. When 'C' is selected, the menu shows wake up source selection for SNVS mode.

Select the wake up source:
Press T for Timer
Press S for switch/button SW7. 

Waiting for key press..

Note:
In this application, DCDC will be turned off durig SP11~SP15. All memories and peripherals in CM7 platform and WAKEUPMIX/DISPLAYMIX/MEGAMIX will be powered down in SP11~SP15.
For ram targets(debug/release), application can't wake up from SP11~SP15 since data in ram will be lost.

Note:
To debug in external flash, following steps are needed:
1. Select the flash target and compile.
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to external flash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
