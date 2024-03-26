Overview
========
This example shows how to use SDK drivers to use the Pin interrupt & pattern match peripheral.

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
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (P6) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

PINT Pattern Match example

PINT Pattern match events are configured

Press corresponding switches to generate events

2.This example configures "Pin Interrupt 0" to be invoked when SW4 switch is pressed by the user. 
Bit slice 0 is configured as an endpoint in sticky falling edge mode. The interrupt callback prints 
"PINT Pin Interrupt 0 event detected. PatternMatch status =        1". 

"Pin Interrupt 2" is configured to be invoked when rising edge on SW4, SW1 is detected. The 
interrupt callback prints "PINT Pin Interrupt 2 event detected. PatternMatch status =     100". Bit slices
1 configured to detect sticky rising edge. Bit slice 2 is configured as an endpoint.


note:if need use secure pint,you just needs to modify base address.Find PINT baseaddr in source file,
	 you need change it to SECPINT. everything else stays the same.Because of SECURE PINT device only 
	 support PORT0,so we only need to config secure pint interrupt 0.
	 For example, the following
	 PINT_Init(PINT);
     PINT_PatternMatchConfig(PINT, kPINT_PatternMatchBSlice0, &pmcfg);
     PINT_EnableCallbackByIndex(PINT,kPINT_PinInt0);
     PINT_PatternMatchEnable(PINT);
	 Change the above four lines into the following:
	 PINT_Init(SECPINT);
     PINT_PatternMatchConfig(SECPINT, kSECPINT_PatternMatchBSlice0, &pmcfg);
     PINT_EnableCallbackByIndex(SECPINT,kPINT_SecPinInt0);
	 PINT_PatternMatchEnable(SECPINT);
	 
	 After config secure pint device, you can see log output "PINT Pin Interrupt 8 event detected. PatternMatch status =        1"
	 by press SW4 button.
