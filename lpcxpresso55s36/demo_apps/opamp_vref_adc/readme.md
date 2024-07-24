Overview
========
This project shows how to use ADC sampling opamp out vaule. In the example opamp,
the positive reference voltage is set as VREF output. The OPAMP output is 1X of VREF
out. When VREF out change, the OPAMP changes accordingly.

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
ADC_channel(J9_7) input connect OPAMP out (TP40).

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
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
When the example run successfully, The following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~
OPAMP VREF LPADC DEMO.
ADC Full Range: 65536

Please input a trim value (0-11):11
Use trim value: 11
Expected voltage on VREF_OUT: 2.100V
Please press any key exluding key (R or r) to get user channel's ADC value.
ADC value: 42566
Actual voltage on OPAMP_OUT: 2.130V
ADC value: 42554
Actual voltage on OPAMP_OUT: 2.130V
ADC value: 42558
Actual voltage on OPAMP_OUT: 2.130V
ADC value: 42570
Actual voltage on OPAMP_OUT: 2.131V
ADC value: 42554
Actual voltage on OPAMP_OUT: 2.130V
ADC value: 42566
Actual voltage on OPAMP_OUT: 2.130V
ADC value: 42554
Actual voltage on OPAMP_OUT: 2.130V

~~~~~~~~~~~~~~~~~~~~~
