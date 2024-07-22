
SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J1) on the target board.
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
The "user key" is: SW1
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
Entering Powerdown [Reset to wakeup] ...
Press any key to confirm to enter the deep sleep mode and wakeup the device by press sw3 key on board.
/* Type in any key into UART terminal */

/* Press the sw3 key on board. */
Gin event occurs
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
The "user key" is: SW1
Select an option
	1. Sleep mode
	2. Deep Sleep mode
	3. power down mode
	4. Deep power down mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
