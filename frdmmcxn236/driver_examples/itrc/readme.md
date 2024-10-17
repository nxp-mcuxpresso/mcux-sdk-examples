Overview
========
The ITRC Example project is a demonstration program that uses the MCUX SDK software to set up Intrusion and Tamper Response Controller.
Then tests the expected behaviour.


SDK version
===========
- Version: 2.16.100

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

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J10) on the board
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
When the example runs successfully, you can see the similar information from the terminal as below.

ITRC Peripheral Driver Example

Pass: No Event/Action triggered in STATUS after Init

Pass: No Event triggered in STATUS1 after Init

Enable ITRC IRQ Action response to SW Event 0

Trigger SW Event 0

ITRC IRQ Reached!
ITRC STATUS:
SW Event0 occured!
Generated ITRC interrupt!

Clear ITRC IRQ and SW Event 0 STATUS

Disable ITRC IRQ Action response to SW Event 0

Trigger SW Event 0

Pass: No Action triggered

End of example
