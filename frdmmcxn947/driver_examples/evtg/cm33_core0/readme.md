Overview
========
The EVTG demo application demonstrates the use of IO and EVTG peripherals.
The example Use the two IO as the input of the EVTG function: EVTG_OUT = (IO0 & IO1).
When IO0 and IO1 are both high level, the EVTG_OUT is high level.
When any of IO is low level, the EVTG_OUT is low level.
The purpose of this demonstration is to show how to use the EVTG driver in the SDK software.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============

```
EVTG_OUT = EXT_TRIG_OUT0 P1_2 J1-14
IO0      = EXT_TRIG_IN0 P1_0 J2-17
IO1      = EXT_TRIG_IN1 P1_1 J2-15
```

Prepare the Demo
================
1. Connect the type-c and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================

The log below shows the output of the EVTG demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EVTG project.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When IO0 and IO1 are both high level, the EVTG_OUT is high level.
When any of IO is low level, the EVTG_OUT is low level.
