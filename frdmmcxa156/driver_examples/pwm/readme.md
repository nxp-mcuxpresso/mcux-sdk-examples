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

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 board
- Personal Computer
- Oscilloscope

Board settings
==============
* Probe the pwm signal using an oscilloscope
 - PWM0_A0 output signal J3-15(PIO3_6).
 - PWM0_B0 output signal J3-13(PIO3_7).
 - PWM0_A1 output signal J3-11(PIO3_8).
 - PWM0_A2 output signal J3-7(PIO3_10).
* Connet J3-8(LDO_3V3) to following pins to set PWM output in fault state
 - TRIG_IN5  input signal J3-1(PIO2_7).
 - TRIG_IN8  input signal J3-3(PIO2_20).
 - TRIG_IN9  input signal J4-3(PIO2_17).
 - TRIG_IN10 input signal J4-5(PIO3_31).

Prepare the Demo
================
1.  Connect a USB Type-C cable between the host PC and the MCU-Link USB port on the target board.
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~
FlexPWM driver example
~~~~~~~~~~~~~~~~~~~~~~~
