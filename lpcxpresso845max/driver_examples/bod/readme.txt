Overview
========
The bod example shows how to use LPC BOD(Brown-out detector) in the simplest way.
To run this example, user should remove the jumper for the power source selector,
and connect the adjustable input voltage to the MCU's Vin pin.
If the input voltage of the Vin pin is lower than the threshold voltage, the BOD interrupt
will be asserted.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Board settings
==============
Remove the SJ6. Connect the adjustable input voltage to JP2-1.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the BOD demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOD INTERRUPT EXAMPLE.
Please adjust input voltage low than 2.66V to trigger BOD interrupt.

BOD interrupt occurred, input voltage is low than 2.66V.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:When the BOD demo is started, the analog voltage input of the JP2_1 pin should be 3.3v. After running the demo, 
adjust the analog voltage input according to the prompts to trigger the BOD interrupt.
