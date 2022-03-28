Overview
========
The INPUT_MUX AOI demo application demonstrates the use of CMP and SCTIMER AOI peripherals.
The example Use the output of cmp and sctimer as the input of the aoi function. When the input voltage of cmp changes,
The output of cmp changes, which changes the output of aoi.  
The purpose of this demonstration is to show how to use the AOI driver in the SDK software.

Run the demo
The VREF voltage is half of the reference voltage.

After the demonstration runs successfully, you can see the following information on the oscilloscope:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CMP input voltage is less than the VREF voltage, AOI has no waveform output
CMP input voltage is greater than the VREF voltage, AOI continuous output square wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
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
VREF connect to 1.6V (3.3V * 15 /31).
The log below shows the output of the aoi demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AOI Demo: Start...

AOI INPUT_MUX driver example

Then change CMP analog input:
If CMP0_D (J44-21)connected to the input voltage that in the range of 0V to 1.5V then the  AOI0_OUT0 (J92-12) output No waveform output.
If CMP0_D (J44-21)connected to the input voltage that in the range of 1.7V to 3.3V then the AOI0_OUT0 (J92-12) continuous output square wave.


AOI INPUT_MUX logic normal!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
