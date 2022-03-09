Overview
========

The DAC / ADC demo application demonstrates the use of the DAC and ADC peripherals. This application demonstrates how to
configure the DAC and set the output on the DAC. This demo also demonstrates how to configure the ADC in 'Blocking Mode'
and how to read ADC values.
The code of this demo has been prepared and updated for use with the MCUXpresso Configuration Tools (Pins/Clocks/Peripherals).

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K66F board
- Personal Computer

Board settings
==============
Connect J4-11(DAC_OUT) to J4-12 (PTB2/ADC0_SE12)

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

This following section shows how to run the demo:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAC ADC Peripheral Demo!

ADC Full Range: XXXX

Press any key to start demo.

Demo begin...
ADC16_DoAutoCalibration() Done.

The user is prompted to enter a voltage to output on the DAC:

Select DAC output level:
        1. 1.0 V
        2. 1.5 V
        3. 2.0 V
        4. 2.5 V
        5. 3.0 V

After entering a valid input, the ADC captures the voltage set by the DAC
and displays the result in the terminal:

Select DAC output level:
        1. 1.0 V
        2. 1.5 V
        3. 2.0 V
        4. 2.5 V
        5. 3.0 V
->3
ADC Value: 2471
ADC Voltage: 1.99
What next?:
        1. Test another DAC output value.
        2. Terminate demo.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At this point, the user can test another DAC output value or terminate the demo.

This configuration exhibits up to 2% error when reading back voltage.
