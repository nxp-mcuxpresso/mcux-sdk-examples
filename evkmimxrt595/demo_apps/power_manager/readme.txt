Overview
========
The power manager demo application demonstrates how to change power modes in the KSDK. The difference between this demo
and power_mode_switch is, this demo uses a notification framework to inform application about the mode change.
Application could register callback to the notification framework, when power mode changes, the callback
function is called and user can do something, such as closing debug console before entering low power mode, and
opening debug console after exiting low power mode.

When this demo runs, the power mode menu is shown in the debug console, where the user can set the MCU to a specific power mode.
User can also set the wakeup source following the debug console prompts.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
- Keep JTAG pins open, don't connect any debugger on it.
- For low power purpose, the demo requires pad voltage supply to be fixed as follows:
  VDDIO_0: 1.8V (PMIC SW2_OUT using default setting 1.8V)
  VDDIO_1: 1.8V (PMIC SW2_OUT using default setting 1.8V)
  VDDIO_2: 1.8V (PMIC SW2_OUT using default setting 1.8V)
  VDDIO_3: 3.3V (PMIC LDO2_OUT using default setting 3.3V)
  VDDIO_4: 1.8V (PMIC SW2_OUT using default setting 1.8V)
  NOTE: Supplying wrong voltage on the pads is harmful to the device.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
The log below shows in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Power Manager Demo.
The "user key" is: SW2
Select an option
        1. Sleep mode
        2. Deep Sleep mode
        3. Deep power down mode
        4. Full deep power down mode
/* Type in '1' in terminal */
Entering Sleep [Press the user key to wakeup] ...
/* Push SW2 */
Pin event occurs
Wakeup.
Select an option
        1. Sleep mode
        2. Deep Sleep mode
        3. Deep power down mode
        4. Full deep power down mode
/* Type in '2' in terminal */
Entering Deep Sleep [Press the user key to wakeup] ...
/* Push SW2 */
Pin event occurs
Wakeup.
Select an option
        1. Sleep mode
        2. Deep Sleep mode
        3. Deep power down mode
        4. Full deep power down mode
/* Type in '3' in terminal */
Entering Deep Powerdown [Reset to wakeup] ...
Press any key to confirm to enter the deep power down mode and wakeup the device by reset.
/* Type in any character to really run into deep power down */
/* Push reset button on board */
Board wake up from deep or full deep power down mode.
Power Manager Demo.
The "user key" is: SW2
Select an option
        1. Sleep mode
        2. Deep Sleep mode
        3. Deep power down mode
        4. Full deep power down mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
