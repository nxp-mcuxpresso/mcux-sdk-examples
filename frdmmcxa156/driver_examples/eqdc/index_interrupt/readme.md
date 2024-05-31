Overview
========

The eqdc_index_interrupt example shows how to use the interrupt of EQDC module with EQDC driver.

In this example, user needs to connect a real encoder to the board. Actually, only PHASE A and PHASE B are enough for the basic application. However, the additional INDEX/HOME/TRIGGER could be used as the event detector. 

INDEX: This pulse can optionally reset the position counter and the pulse accumulator of the quadrature decoder module. It also causes a change of state on the revolution counter. The direction of this change, increment or decrement, is calculated from the PHASEA and PHASEB inputs.

HOME: This input can be used to trigger the initialization of the position counters. Often this signal is connected to a sensor signalling the motor or machine, sending notification that it has reached a defined home position.

TRIGGER: This input can be used to clear the position counters or to take a snapshot of the POS, REV, and POSD registers. Often this signal is connected to a periodic pulse generator or timer to indicate an elapsed time period.

This example uses INDEX to response the external event. When running the project, user can turn the encoder so that EQDC module can monitor the position change. Also, a variable counter would count the time of INDEX interrupt for rising edge on INDEX signal line.


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
- An encoder with PHASE A/B signals.

Board settings
==============
1. For pin connection between board and encoder,
      J4_5 -> kINPUTMUX_TrigIn10ToQdc0Phasea -> EQDC_PHA
      J4_3 -> kINPUTMUX_TrigIn9ToQdc0Phaseb -> EQDC_PHB
      J4_1 -> kINPUTMUX_TrigIn4ToQdc0Index -> EQDC_INDEX


Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the MCU-Link USB port on the target board.
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
Turn the encoder feed the negative pulse into INDEX and type in any key into terminal.
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

EQDC INDEX Interrupt Example.
Press any key to get the encoder values ...

Current position value: 0
Current position differential value: 0
Current position revolution value: 0
g_EqdcIndexCounter: 0

Current position value: 20
Current position differential value: 20
Current position revolution value: 1
g_EqdcIndexCounter: 1

Current position value: 25
Current position differential value: 5
Current position revolution value: 2
g_EqdcIndexCounter: 2

...
