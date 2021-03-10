## Overview
-----------
The dac_dma example shows how to use DAC with dma and produce an arbitrary, user-defined waveform of
selectable frequency.The output can be observed with an oscilloscope. 

When the DAC's double-buffer feature is enabled, any write to the CR register will only load the pre-buffer, which
shares its register address with the CR register. The CR itself will be loaded from the pre-buffer whenever the 
counter reaches zero and the DMA request would be raised. At the same time the counter is reloaded with the COUNTVAL
register value. user-defined waveform array would be transfered to pre-buffer in order by DMA.

## Functional Description
-------------------------
In this example, The DAC will output the voltage received from serial terminal.

When start running this example, main routine will initialize clock, pin mux configuration,
and configure the DAC module to make it work in dma mode. user-defined waveform array would
be transfered to pre-buffer in order by DMA. Finally, we can probe the signal from DAC_OUT pin
by using an oscilloscope.

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
J6-5 is DAC0 output pin.

## Run the Project
------------------------
Run the example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J4 on the 
   LPCXpresso845MAX board).

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Launch the debugger in your IDE to begin running the project.

5. Input several characters according to prompt information displayed in terminal. 

## Expected Result
------------------------
Probe the signal from DAC_OUT(J6-5) pin by using an oscilloscope and see triangular wave.
