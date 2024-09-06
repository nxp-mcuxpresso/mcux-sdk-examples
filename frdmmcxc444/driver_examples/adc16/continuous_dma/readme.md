Overview
========
The ADC16 continuous DMA demo application demonstrates the usage of the ADC and DMA peripheral while in a continuous mode. The
ADC16 is first set to continuous mode. In continuous convert configuration, only the initial rising-edge to launch continuous conversions is
observed, and until conversion is aborted, the ADC16 continues to do conversions on the same SCn register that initiated the conversion. 
DMA request will be asserted during an ADC16 conversion complete event noted when any of the SC1n[COCO] flags is asserted. DMA will transfer
ADC16 results to memory and if users press any key, demo will average ADC16 results stored in memory and print average on the terminal.  

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
3.  Connect external signal source J6-2 to user-defined channel, defined by "DEMO_ADC16_CHANNEL" in source code.
    Shorten jumper JP6 to use external reference voltage pair pins.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

ADC16 CONTINUOUS DMA DEMO
ADC16_DoAutoCalibration() Done.
ADC Full Range: XXXX
Press any key to get user channel's ADC value ...
ADC value: 32878
ADC value: 34463
ADC value: 36226
ADC value: 32819
ADC value: 37477

