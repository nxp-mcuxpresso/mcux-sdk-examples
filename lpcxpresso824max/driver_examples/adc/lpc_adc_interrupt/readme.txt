## Overview
-----------
The lpc_adc_interrupt example shows how to use interrupt with LPC ADC driver.

In this example, an outside input is used to created the input analog signal. 
When user type in any key from the keyboard, the software trigger API is called to start the conversion. 
Then it polls the flag variable which would be asserted when the conversion completed ISR is executed. 
Then just print the conversion result to terminal.

## Functional Description
-------------------------
1.This example demonstrates how to configure the A sequences with interrupt, assigning one channel with software
  trigger, you can configure channel via "DEMO_ADC_SAMPLE_CHANNEL_NUMBER".
  
2.Before configuration of the ADC begins, the ADC is put through a self-calibration cycle.  

3.Enable the Conversion-Complete or Sequence-Complete interrupt for sequences A.
  
4.After ADC channels are assigned to each of the sequences, if the user enters any key via terminal, software trigger will start.  
  
5.When the conversion completes, the interrupt would be triggered. The ISR will print conversion result to terminal.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso824MAX board
- Personal Computer

## Board Settings
------------------------
- ADC CH1 input signal J5-1(PIO0-6).

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J3 on the 
   LPCXpresso824MAX board) for receiving debug information.

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).




4. Monitor the information on the debug console.

## Expected Result
------------------------
Press any key and print user channel's result in serial terminal.

