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
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93AUTO-EVK board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW2 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J26 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Boot Linux BSP to u-boot, and load M core image from SD card to run. (Put the image into SD card before.)
    => load mmc 1:1 0x80000000 imx93_m33_TCM_power_mode_switch.bin
    => cp.b 0x80000000 0x201e0000 0x20000
    => bootaux 0x1ffe0000 0
5.  Append "${mcore_clk}" in u-boot "mmcargs" env, before booting linux.
    => run prepare_mcore
    => setenv mmcargs 'setenv bootargs ${jh_clk} ${mcore_clk} console=${console} root=${mmcroot}'
6.  You can also test power_mode_switch function with flash.bin. After flash.bin compilation, please load the bianry by using "UUU" command.
    For flash.bin test mode, please skip step 4.

Running the demo
================
Mcore debug console shows the menu to command the MCU to the target power mode. The target power mode can be wakeup by LPTMR and LPUART.
SYSTEM SLEEP flow: In linux console use command "echo mem > /sys/power/state" to suspend A55, then select SUSPEND mode in Mcore debug console. The STBY LED on board will light with RED color.
NOTE: M33 wakeup A55 by using MU interrupt "GCR[GIR1]". Please ensure the A55 already in SUSPEND mode, then press "W" button in Mcore debug console. Or you will meet call trace info in linux.

The log below shows the output of the power mode switch demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start SRTM communication
Task 1 is working now

####################  Power Mode Switch Task ####################

    Build Time: Feb  3 2023--21:35:13
    Core Clock: 200000000Hz

Select the desired operation

Press  A to enter: Normal RUN mode
Press  B to enter: WAIT mode
Press  C to enter: STOP mode
Press  D to enter: SUSPEND mode
Press  W to wakeup A55 core

Waiting for power mode select..
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
