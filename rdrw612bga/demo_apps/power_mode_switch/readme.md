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
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
Ensure R148 is removed.
Ensure R520 is removed, R518 is installed to make SW4 work.
U38 DIP 1,2,3,4 all off
HD12 1-2, 3-4 connected
JP31 1-2 connected
JP34 2-3 connected
JP39 2-3 connected

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When running the demo, the debug console shows the menu to command the MCU to the target power mode (PM0 - Active, PM1 - Idle, PM2 - Standby, PM3 - Sleep, PM4 - Deep Sleep).
NOTE: Only input when the demo asks for input. Input entered at any other time might cause the debug console to overflow and receive the wrong input value.

Here's the initial menu when running successfully.
~~~~~~~~~~~~~~~~~~~~~
MCU wakeup source 0x0...

####################  Power Mode Switch ####################

    Build Time: Mar  2 2022--10:11:47
    Core Clock: 260000000Hz

Select the desired operation

Press 0 for enter: PM0 - Active
Press 1 for enter: PM1 - Idle
Press 2 for enter: PM2 - Standby
Press 3 for enter: PM3 - Sleep
Press 4 for enter: PM4 - Deep Sleep

Waiting for power mode select..

1
Select the wake up source:
Press T for RTC.
Press 1 for wakeup pin1(SW4).
Press U for UART wakeup.

Waiting for key press..

t
Select the wake up timeout in seconds.
The allowed range is 1s ~ 99s.
Eg. enter 05 to wake up in 5 seconds.

Waiting for input timeout value...

03
RTC wake up after 3 seconds.
Woken up by RTC
Exit from power mode 1

####################  Power Mode Switch ####################

    Build Time: Mar  2 2022--10:11:47
    Core Clock: 260000000Hz

Select the desired operation

Press 0 for enter: PM0 - Active
Press 1 for enter: PM1 - Idle
Press 2 for enter: PM2 - Standby
Press 3 for enter: PM3 - Sleep
Press 4 for enter: PM4 - Deep Sleep

Waiting for power mode select..

3
Select the wake up source:
Press T for RTC.
Press 1 for wakeup pin1(SW4).

Waiting for key press..

1
Push wakeup PIN1 to wake up.

MCU wakeup source 0x0...

####################  Power Mode Switch ####################

    Build Time: Mar  2 2022--10:11:47
    Core Clock: 260000000Hz

Select the desired operation

Press 0 for enter: PM0 - Active
Press 1 for enter: PM1 - Idle
Press 2 for enter: PM2 - Standby
Press 3 for enter: PM3 - Sleep
Press 4 for enter: PM4 - Deep Sleep

Waiting for power mode select..

4
Select the wake up source:
Press T for RTC.
Press 1 for wakeup pin1(SW4).

Waiting for key press..

t
Select the wake up timeout in seconds.
The allowed range is 1s ~ 99s.
Eg. enter 05 to wake up in 5 seconds.

Waiting for input timeout value...

02
RTC wake up after 2 seconds.

MCU wakeup source 0x0...

####################  Power Mode Switch ####################

    Build Time: Mar  2 2022--10:11:47
    Core Clock: 260000000Hz

Select the desired operation

Press 0 for enter: PM0 - Active
Press 1 for enter: PM1 - Idle
Press 2 for enter: PM2 - Standby
Press 3 for enter: PM3 - Sleep
Press 4 for enter: PM4 - Deep Sleep

Waiting for power mode select..
~~~~~~~~~~~~~~~~~~~~~
