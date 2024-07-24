Overview
========
The freqme_interrupt is a demonstration program of the SDK LPC_FREQME driver's features.
The example demostrates the usage of frequency-measurement operating mode and pulse-width measurement operating mode.
In frequency measurement mode, the reference clock source is fixed. Users can select target clock source and input
reference clock scaling factor, then after a while, the frequency of target clock source will be printed to the
terminal. To measure the frequency with a high degree of accuracy, the frequency of target clock is better less than
that of the reference clock.
In pulse width measurement mode, the target clock source is fixed. Users can select reference clock source and pulse
polarity, then after a while, the high or low period of reference clock will be printed to the terminal. To measure the
pulse period with a high degree of accuracy, the frequency of reference clock is better less than that of target clock.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
To measure the frequency or pulse width of external input clock, please connect input clock signal to
J9-6(FREQME_GPIO_CLK_A) or J9-5(FREQME_GPIO_CLK_B)


Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J17) on the board
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
The log below shows the output of the freqme_interrupt demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FREQME Interrupt Example!
Please select operate mode...
        A -- Frequency Measurement Mode.
        B -- Pulse Width Measurement Mode.
Frequency Measurement Mode Selected!
Please select the target clock:
                A -- XTAL32MHz
                B -- FRO_OSC_12M
                C -- FREQME_GPIO_CLK_A
                D -- FREQME_GPIO_CLK_B
Please input the scale factor of reference clock(Ranges from 0 to 31).
20
Target clock frequency is 11999725 Hz.
Please select operate mode...
        A -- Frequency Measurement Mode.
        B -- Pulse Width Measurement Mode.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
