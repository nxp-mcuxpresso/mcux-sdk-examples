Overview
========
The LPCMP interrupt Example shows how to use interrupt with LPCMP driver.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
LPCMP's positive channel input. On the negative side, the internal DAC is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user-defined channel's voltage crosses the internal DAC's value. The change of
comparator's output would generate the falling and rising edge events with their interrupts enabled. When any LPCMP 
interrupt happens, the LPCMP's ISR would turn on the LED light if detecting the output's rising edge, or turn off it when
detecting the output's falling edge.

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
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
This example project uses LPCMP0 to compare the voltage signal input from channel 0(P1_0)
with the voltage signal(half of VREF_OUT) output by LPCMP's internal DAC.
The voltage signal input from P1_0 corresponding J7-3 on FRDM board should be changed
to observe the dynamical IO state. You will see the red led toggle with the voltage change.

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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPCMP Interrupt Example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

