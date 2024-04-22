Overview
========

The dac12_basic example shows how to use DAC12 module simply as the general DAC12 converter.

When the DAC12's fifo feature is not enabled, Any write to the DATA register will replace the
data in the buffer and push data to analog conversion without trigger support.
In this example, it gets the value from terminal,
outputs the DAC12 output voltage through DAC12 output pin.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer
- Multimeter

Board settings
==============
- Connect J10 pin 1-2.

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
The log below shows in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DAC basic Example.

Please input a value (0 - 4095) to output with DAC:
Input value is 400
DAC out: 400

Please input a value (0 - 4095) to output with DAC:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Then user can measure the DAC0 output pin(TP9) to check responding voltage = 1.8v * 400 / 4096
