Overview
========

The adc16_interrupt example shows how to use interrupt with ADC16 driver.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the ADC16's
sample input. When running the project, typing any key into debug console would trigger the conversion. ADC interrupt 
would be asserted once the conversion is completed. In ADC ISR, the conversion completed flag would be cleared by 
reading the conversion result value. Also, the conversion result value is stored, and the ISR counter is increased. 
These information would be printed when the execution return to the main loop.

The point is that the ADC16 interrupt configuration is set when configuring the ADC16's conversion channel. When in 
software trigger mode, the conversion would be launched by the operation of configuring channel, just like writing a
conversion command. So if user wants to generate the interrupt every time the conversion is completed, the channel's 
configuration with enabling interrupt setting would be used for each conversion.

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
- FRDM-MCXC444 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Connect external signal source J6-2 to user-defined channel, defined by "DEMO_ADC16_USER_CHANNEL" in source code.
    Shorten jumper JP6 to use external reference voltage pair pins.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

ADC16 interrupt Example.
ADC16_DoAutoCalibration() Done.
ADC Full Range: XXXX
Press any key to get user channel's ADC value ...
ADC Value: 1918
ADC Interrupt Count: 1
ADC Value: 1635
ADC Interrupt Count: 2
ADC Value: 1412
ADC Interrupt Count: 3
ADC Value: 1332
ADC Interrupt Count: 4
ADC Value: 1543
ADC Interrupt Count: 5
ADC Value: 1579
ADC Interrupt Count: 6
