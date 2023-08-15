Overview
========
The AFE Example project is a demonstration program that uses the KSDK software.
In this case the example provide two functions. Measure the voltage difference two pin of the
channel 2 and 3. And monitor AC frequency on two pins of channel 2.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
- Personal Computer

Board settings
==============
On TWR-KM35Z75M board:
- Use external power supply to input DC analog signals to AFE channel 2(J31-pin10 & J31-pin12)
and AFE channel 3(J31-pin14 & J31-pin16), AFE sample can support both single-ended or differential mode;
- To monitor frequency, use a signal generator to input the AC sine signal to the
AFE channel 2(J31-pin10 & J31-pin12).

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
These instructions are displayed/shown on the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~
AFE QTIMER DEMO.

First Channel value: 8388607

Second Channel value: 90678

Frequency: 55
Frequency: 55
Frequency: 55
Frequency: 55
Frequency: 55
Frequency: 54
Frequency: 55
Frequency: 55
Frequency: 55

AFE QTIMER DEMO FINISH.
~~~~~~~~~~~~~~~~~~~~~~~
