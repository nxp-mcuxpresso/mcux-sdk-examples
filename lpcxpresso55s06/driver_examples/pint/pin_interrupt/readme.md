Overview
========
This example shows how to use SDK drivers to use the Pin interrupt & pattern match peripheral.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J1) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

PINT Pin interrupt example

PINT Pin Interrupt events are configured

Press corresponding switches to generate events

2. This example configures "Pin Interrupt 0" to be invoked when SW4 switch is pressed by the user.
   The interrupt callback prints "PINT Pin Interrupt 0 event detected". "Pin Interrupt 1" is
   is configured to be invoked when SW1 is pressed. The interrupt callback prints "PINT Pin Interrupt 
   1 event detected". "Pin Interrupt 2" is configured to be invoked when SW3 is pressed. The interrupt 
   callback prints "PINT Pin Interrupt 2 event detected".

note:if need use secure pint,you just needs to modify base address.Find PINT baseaddr in source file,
	 you need change it to SECPINT. everything else stays the same.Because of SECURE PINT device only 
	 support PORT0,so we only need to config secure pint interrupt 0.
	 For example, the following
	 PINT_Init(PINT);
	 PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, pint_intr_callback);
	 PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
	 Change the above three lines into the following:
	 PINT_Init(SECPINT);
	 PINT_PinInterruptConfig(SECPINT, kPINT_SecPinInt0, kPINT_PinIntEnableRiseEdge, pint_intr_callback);
	 PINT_EnableCallbackByIndex(SECPINT, kPINT_SecPinInt0);
	 
	 After config secure pint device, you can see log output "The interrupt callback prints "PINT Pin Interrupt 8 event detected"
	 by press SW4 button.
	 
