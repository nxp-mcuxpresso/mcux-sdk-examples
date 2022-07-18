Overview
========
The TRGMUX project is a simple demonstration program of the SDK TRGMUX driver. It generates
a 10KHz PWM by LPIT0 CH0, and clone by TRGMUX to output this PWM to eight TRGMUX_OUTx pins.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE17Z board
- Personal Computer

Board settings
==============
The example requires connection between the trgmux pin and oscilloscope.
Connect each of the trgmux output pin with the anode of oscilloscope:
J1-5(FRDM-KE17Z board) -> Anode of oscilloscope.
Or J1-7(FRDM-KE17Z board) -> Anode of oscilloscope.
Or J1-2 (FRDM-KE17Z board) -> Anode of oscilloscope.
Or J1-4 (FRDM-KE17Z board) -> Anode of oscilloscope.
Or J4-1(FRDM-KE17Z board) -> Anode of oscilloscope.
Or J2-2(FRDM-KE17Z board) -> Anode of oscilloscope.
Or J4-7(FRDM-KE17Z board) -> Anode of oscilloscope.
Or J4-5 (FRDM-KE17Z board) -> Anode of oscilloscope.
GND  (FRDM-KE17Z board) -> Cathode of oscilloscope.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
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
When the example runs successfully, you can see the 10KHz PWM on each of the 8 pins.
