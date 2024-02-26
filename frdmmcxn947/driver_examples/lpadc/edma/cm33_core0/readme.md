Overview
========

The lpadc_edma example shows how to use ADC to trigger a DMA transfer. 

In this example, user should indicate a channel to provide a voltage signal(can be controlled by user) as the LPADC's sample input
and active the low power timer to trigger the ADC conversion upon the set Time reached(can be changed by user). A programmable watermark
threshold supports configurable notification of data availability. When FCTRLn[FCOUNT] is greater than FCTRLn[FWMARK], the associated RDY
flag is asserted which would generate a DMA request and trigger the eDMA to move the ADC conversion result from ADC result FIFO to user
indicated memory. Then the major loop waits for the transfer to be done. The results would be printed on terminal when any key pressed.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the type-c and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.
5. The J8 Pin 28 (ADC0_A2) on EVK board is used to monitor the voltage.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPADC EDMA Example
Configuring LPADC...
ADC Full Range: 4096
Configuring LPADC EDMA...
Press any key to print output buffer:

0 = 3192
1 = 3192
2 = 3184
3 = 3184
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

