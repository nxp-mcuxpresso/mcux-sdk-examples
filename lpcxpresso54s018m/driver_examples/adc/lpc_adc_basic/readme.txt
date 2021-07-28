Overview
========

The lpc_adc_basic example shows how to use LPC ADC driver in the simplest way.

In this example, the sensor is used to created the input analog signal. 
When user type in any key from the keyboard, the software trigger API is called to start the conversion. 
Then it polls the conversion sequence A's flag till the conversion is completed. When the conversion is 
completed, just print the conversion result to terminal.

Project Information
1.what are ADC conversion sequences?
  A conversion sequence is a single pass through a series of A/D conversions performed on a selected set of
  A/D channels. Software can set-up two independent conversion sequences, either of which can be triggered 
  by software or by a transition on one of the hardware triggers. Each sequence can be triggered by a different 
  hardware trigger. One of these conversion sequences is referred to as the A sequence and the other as the B
  sequence. It is not necessary to employ both sequences. An optional single-step mode allows advancing through
  the channels of a sequence one at a time on each successive occurrence of a trigger. The user can select whether
  a trigger on the B sequence can interrupt an already-in-progress A sequence. The B sequence, however, can never be
  interrupted by an A trigger.
  
2. How to use software-triggered conversion?
   There are two ways that software can trigger a conversion sequence:
      1. Start Bit: The first way to software-trigger an sequence is by setting the START bit in
         the corresponding SEQn_CTRL register. The response to this is identical to
         occurrence of a hardware trigger on that sequence. Specifically, one cycle of
         conversions through that conversion sequence will be immediately triggered except
         as indicated above. (In this example, we use this way.)
      2. Burst Mode: The other way to initiate conversions is to set the BURST bit in the
         SEQn_CTRL register. As long as this bit is 1 the designated conversion sequence will
         be continuously and repetitively cycled through. Any new software or hardware trigger
         on this sequence will be ignored.
  
Program Flow
1.This example demonstrates how to configure the A sequences with polling, assigning one channel with software
  trigger, you can configure channel via "DEMO_ADC_SAMPLE_CHANNEL_NUMBER".
  
2.Before configuration of the ADC begins, the ADC is put through a self-calibration cycle.  
  
3.After ADC channels are assigned to each of the sequences, the software trigger is chosen. Setting 
  SEQA_CTRL_START to '1' will trigger sequence A.
  
4.After ADC channels are assigned to each of the sequences, if the user enters any key via terminal, software trigger will start. 

5.Read the corresponding DATAVALID field with polling to judge whether the conversion completes and the result is ready.
  If the result is ready, the example will printf result information to terminal.

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018M board
- Personal Computer

Board settings
==============
- ADC CH4 input signal J12-2(PIO0-16).

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
This demo is an external flash plain load demo, ROM will copy image in external flash to SRAMX to run:
1. Build the demo to generate a bin file.
   Note: If need to generate bin file using MCUXpresso IDE, below steps need to be followed:
         Set in example Properties->C/C++ Build->Settings->Build steps->Post-build steps->Edit
         enbable arm-none-eabi-objcopy -v -O binary "&{BuildArtifactFileName}" "&{BuildArtifactFileBaseName}.bin" 
         
         This plainload example linked the vector table to 0x00000000, but program to external flash 0x10000000.

2. Program the bin file to external on board flash via SEGGER J-FLASH Lite(V6.22 or higher):

   a. Open SEGGER J-FLASH Lite, select device LPC54S018M.

   b. Click the 'Erase Chip' to erase the extrenal flash.(if can not success, press SW4 button and reset the board, and try to erase again)

   c. Select the bin data file, set the '.bin/Erase Start' address to 0x10000000, then click 'Program Device'
Note: Please use above way to program the binary file built by armgcc tool chain to external flash. 
      For IAR, KEIL, MCUXpresso IDE, you can use the IDE tool to program the external flash.  
Press any key from keyboard and trigger the conversion.
The log below shows example output of the ADC basic example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ADC basic example.
ADC Calibration Done.
Configuration Done.
ADC Full Range: XXXX
adcResultInfoStruct.result        = 675
adcResultInfoStruct.channelNumber = 4
adcResultInfoStruct.overrunFlag   = 0

adcResultInfoStruct.result        = 676
adcResultInfoStruct.channelNumber = 4
adcResultInfoStruct.overrunFlag   = 0

adcResultInfoStruct.result        = 675
adcResultInfoStruct.channelNumber = 4
adcResultInfoStruct.overrunFlag   = 0

...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
