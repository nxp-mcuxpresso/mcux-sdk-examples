Overview
========
The power_manager_lpc application shows the usage of normal power mode control APIs for entering the three kinds of
low power mode: Sleep mode, Deep Sleep mode and Sleep Power Down mode. When the application runs to each low power
mode, the device would cut off the power for specific modules to save energy. The device can be also waken up by
prepared wakeup source from external event.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.
 - In order to meet typedef power consumption of DateSheet manual, Please configure MCU under the following conditions.
     • Configure all pins as GPIO with pull-up resistor disabled in the IOCON block.
     • Configure GPIO pins as outputs using the GPIO DIR register.
     • Write 1 to the GPIO CLR register to drive the outputs LOW.
     • All peripherals disabled.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S69 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port P6) on the target board.
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
The log below shows in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Power Manager Demo for LPC device.
The "user key" is: SW3
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
/* Type in '1' into UART terminal */
Entering Sleep [Press the user key to wakeup] ...
/* Press the user key on board */
Pin event occurs
Wakeup.
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
/* Type in '2' into UART terminal */
Entering Deep Sleep [Press the user key to wakeup] ...
/* Press the user key on board */
Pin event occurs
Wakeup.
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
/* Type in '3' into UART terminal */
Entering Deep Powerdown [Reset to wakeup] ...
Press any key to confirm to enter the deep sleep mode and wakeup the device by press s1 key on board.
/* Type in any key into UART terminal */

/* Press the s1 key on board. */
Wakeup.
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
/* Type in '4' into UART terminal */
Entering Deep Powerdown [Reset to wakeup] ...
Press any key to confirm to enter the deep power down mode and wakeup the device by reset.
/* Type in any key into UART terminal */
Please input the number of second to wait for waking up
The second must be positive value
/* Type in 3 into UART terminal */
System will wakeup at 3s later
Power Manager Demo for LPC device.
The "user key" is: SW3
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
