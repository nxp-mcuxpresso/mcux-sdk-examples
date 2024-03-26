Overview
========

The HSCMP interrupt Example shows how to use interrupt with HSCMP driver.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
HSCMP's positive channel input. On the negative side, the internal 6-bit DAC is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user-defined channel's voltage crosses the internal DAC's value. The change of
comparator's output would generate the falling and rising edge events with their interrupts enabled. When any HSCMP 
interrupt happens, the HSCMP's ISR would turn on the LED light if detecting the output's rising edge, or turn off it when
detecting the output's falling edge.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- Personal Computer
- LPCXpresso55S36 board

Board settings
==============
This example project uses HSCMP to compare the voltage signal input from channel 3(P1_5)
with the voltage signal(half of VREF_OUT) output by HSCMP's internal DAC and output different result.
LED_RED will turn ON/OFF corresponding to different result. So the voltage signal input from P1_28
should be changed to observe the dynamical LED state.
- Input signal to J9-9

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
HSCMP Interrupt Example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

