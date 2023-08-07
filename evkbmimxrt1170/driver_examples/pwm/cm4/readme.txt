Overview
========
The PWM project is a simple demonstration program of the SDK PWM driver.
The pulse width modulator (PWM) module contains PWM submodules, each of which is set up to control a single half-bridge power stage.
Fault channel support is provided. This PWM module can generate various switching patterns, including highly sophisticated waveforms.
It can be used to control all known Switched Mode Power Supplies (SMPS) topologies.

The project uses eFlexPWM to generate A-phase pwm, B-phase pwm and C-phase pwm through submodule 0, submodule 1, submodule 2.

Submodule 0 can generate one complementary PWM: PWM A and PWM B. For PWM A, PWM A(1kHz 50% duty cycle) is setup, PWM A works
based on SignedCenterAligned. PWM B operates in PWM A complementary mode, so the dutycycle field of PWM B does not matter.
To prevent short circuits, the dead time count is set, about 650ns.

Submodule 1 and submodule 2 are set to MasterSync. This means that submodule 1 and submodule 2 are initialized synchronously
from submodule 0. In MasterSync mode, submodule 0 can be used as the main module to generate control signals to control
other modules.

This program is set to center-aligned PWM mode, and the duty cycle and position of the PWM can be changed by modifying the
value of pwmVal in this mode.

In this project, the duty cycle of submodule 0 is pwmVal, the duty cycle of submodule 1 is pwmVal divided by 2 and the duty
cycle of submodule 2 is pwmVal divided by 4.

PWM_SetPwmLdok() can issue the load command to multiple submodules at the same time. The values are loaded immediately if
kPWM_ReloadImmediate option was choosen during config. Else the values are loaded at the next PWM reload point.

The log below shows example output of the PWM driver demo in the oscilloscope :

  ^
  | +--------+
  | |        |
  +-+--------+----->     submodule0
  |   +---+
  |   |   |
  +---+---+-------->     submodule1
  |    +-+
  |    | |
  +----+-+--------->     submodule2

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- Oscilloscope

Board settings
==============
* Probe the pwm signal using an oscilloscope
 - At J25-7
 - At J25-11
 - At J25-13
 - At J25-15

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~
FlexPWM driver example
~~~~~~~~~~~~~~~~~~~~~~~
