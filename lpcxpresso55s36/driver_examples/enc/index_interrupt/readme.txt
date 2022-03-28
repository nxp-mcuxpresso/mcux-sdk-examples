Overview
========

The enc_index_interrupt example shows how to use the interrupt of ENC module with ENC driver.

In this example, user needs to connect a real encoder to the board. Actually, only PHASE A and PHASE B are enough for the basic application. However, the additional INDEX/HOME/TRIGGER could be used as the event detector. 

INDEX: This pulse can optionally reset the position counter and the pulse accumulator of the quadrature decoder module. It also causes a change of state on the revolution counter. The direction of this change, increment or decrement, is calculated from the PHASEA and PHASEB inputs.

HOME: This input can be used to trigger the initialization of the position counters. Often this signal is connected to a sensor signalling the motor or machine, sending notification that it has reached a defined home position.

TRIGGER: This input can be used to clear the position counters or to take a snapshot of the POS, REV, and POSD registers. Often this signal is connected to a periodic pulse generator or timer to indicate an elapsed time period.

This example uses INDEX to response the external event. When running the project, user can turn the encoder so that ENC module can monitor the position change. Also, a variable counter would count the time of INDEX interrupt for rising edge on INDEX signal line.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S36 board
- Personal Computer
- An encoder with PHASE A/B signals, optionally with INDEX signal.

Board settings
==============
1. For pin connection between board and encoder,
      J10.3 -> kINPUTMUX_ExtTrigIn3ToEnc0Phasea -> ENC_PHA
      J10.1 -> kINPUTMUX_ExtTrigIn2ToEnc0Phaseb -> ENC_PHB
      J12.3 -> kINPUTMUX_ExtTrigIn8ToEnc0Index  -> ENC_INDEX

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Connect the wires between encoder and the MCU board. See to the code for pin mux setting in function "BOARD_InitPins()".
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bits
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
Turn the encoder feed the negative pulse into INDEX and type in any key into terminal.
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

ENC INDEX Interrupt Example.
Press any key to get the encoder values ...

Current position value: 0
Current position differential value: 0
Current position revolution value: 0
g_EncIndexCounter: 0

Current position value: 20
Current position differential value: 20
Current position revolution value: 1
g_EncIndexCounter: 1

Current position value: 25
Current position differential value: 5
Current position revolution value: 2
g_EncIndexCounter: 2

...



