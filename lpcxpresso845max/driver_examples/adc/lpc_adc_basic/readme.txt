## Overview
-----------
The lpc_adc_basic example shows how to use LPC ADC driver in the simplest way.

In this example, an outside input is used to created the input analog signal. 
When user type in any key from the keyboard, the software trigger API is called to start the conversion. 
Then it polls the conversion sequence A's flag till the conversion is completed. When the conversion is 
completed, just print the conversion result to terminal.

## Functional Description
-------------------------
1.This example demonstrates how to configure the A sequences with polling, assigning one channel with software
  trigger, you can configure channel via "DEMO_ADC_SAMPLE_CHANNEL_NUMBER".
  
2.Before configuration of the ADC begins, the ADC is put through a self-calibration cycle.  
  
3.After ADC channels are assigned to each of the sequences, the software trigger is chosen. Setting 
  SEQA_CTRL_START to '1' will trigger sequence A.
  
4.After ADC channels are assigned to each of the sequences, if the user enters any key via terminal, software trigger will start. 

5.Read the corresponding DATAVALID field with polling to judge whether the conversion completes and the result is ready.
  If the result is ready, the example will printf result inforamtion to terminal.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso845MAX board
- Personal Computer

## Board Settings
------------------------
- ADC CH0 input signal J1-6(PIO0-7).

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the 
   LPCXpresso845MAX board) for receiving debug information.

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

