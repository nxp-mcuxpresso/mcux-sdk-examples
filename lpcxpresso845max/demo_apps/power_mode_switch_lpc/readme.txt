## Overview
-----------------------------------------------------------------------------------------
The power_mode_switch_lpc application shows the usage of normal power mode control APIs for entering the four kinds of
low power mode: Sleep mode, Deep Sleep mode and Power Down mode, deep power down mode. When the application runs to each low power
mode, the device would cut off the power for specific modules to save energy. The device can be also waken up by
prepared wakeup source from external event.

## Functional description
-----------------------------------------------------------------------------------------
This demo is to show how the various power mode can switch to each other.
There is one known issue that terminal may receive mess code after enter deep power down mode, it is caused by 
the uart TX pin state changed by the MCU under deep power down mode. As the RM says, all the functional pins are 
tri-stated in deep power down mode except for the WAKEUP pin and the RESET pin. So the state change will be captured by
the virtual com port as mess code. Software has no workaround for the issue. 

## Toolchain supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Mini/micro USB cable
- LPCXpresso845MAX board
- Personal Computer

## Board Settings
------------------------
No special settings are required.

## Run the Demo
------------------------
1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the board).

2. Open a serial terminal with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Choose an IDE, building the project and download the program to the target board.
   More information about how to compile and program the project can refer to the 

   [Getting Started with MCUXpresso SDK](../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running the demo.

## Expected Result
------------------------
Power mode switch Demo for LPC8xx.

Selecct an option

	1. Sleep mode

	2. Deep Sleep mode

	3. Power Down mode

	4. Deep power down mode
/* after select power mode, terminal will output */
Select wakeup source

    1. Wkt timer

    2. SW2, wakeup key

    3. SW3, reset key
/* after wakeup, terminal will output */
Wakeup.


