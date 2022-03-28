Overview
========

The enc_basic example shows how to quickly start using ENC driver.

In this example, user needs to connect a real encoder to the board. Actually, only PHASE A and PHASE B are enough for the basic application. When running the project, user can turn the encoder so that ENC module can monitor the position change. Then, the internal counter would also count for the position. User can also type keys into terminal, and the current position values recorded by ENC would display. 

The ENC hardware is created with a special synchronize mechanism. There are actually 4 counters (the 32-bit position counter is combined with the two 16-bit counter registers) for position with responding hold registers. When any of the counter registers is read, the contents of each counter register is written to the corresponding hold register. Taking a snapshot of the counters' values provides a consistent view of a system position and a velocity to be attained.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S36 board
- Personal Computer
- An encoder with PHASE A/B signals.

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
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Turn the encoder and type in any key into terminal.
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

ENC Basic Example.
Press any key to get the encoder values ...

Current position value: 0
Position differential value: 0
Position revolution value: 0

Current position value: 10
Position differential value: 10
Position revolution value: 0

...


