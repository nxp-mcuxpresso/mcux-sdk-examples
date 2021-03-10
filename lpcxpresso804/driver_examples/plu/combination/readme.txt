## Overview
-----------------------------------------------------------------------------------------
The PLU combination project is a simple demonstration program of the SDK PLU driver. In this example, a number of 
GPIO pins are connected to PLU inputs and the LED is used to monitor the PLU outputs to demonstrate the 
configuration and use of the Programmable Logic Unit (PLU) to construct a combinational logic network.

## Functional Description
-----------------------------------------------------------------------------------------
The example will configure the SWM for PLU inputs, outputs, and clock input, then configure the PLU module 
to make it work. 

Note: The RGB LED in this case turns on when the pin level is low and it turns off when the pin level is high.

Each of the three LUT elements are connected to the three PLU input sources to control the RGB LED.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Mini/micro USB cable
- LPCXpresso804 board
- Personal Computer

## Board Settings
------------------------
PLU input pins connection:
- CN3_7 (GPIO0_8) to CN8_2 (PLU_IN2).
- CN3_6 (GPIO0_9) to CN3_10 (PLU_IN3).
- CN5_1 (GPIO0_10) to CN8_5 (PLU_IN4).

PLU output pins connection:
- CN3_1 (PLU_OUT0) to CN8_4 (LED_RED).
- CN3_8 (PLU_OUT1) to CN8_6 (LED_GREEN).
- CN5_2 (PLU_OUT2) to CN8_3 (LED_BLUE).

## Run the Project
------------------------
1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN2 on the board).

2. Open a serial terminal with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Choose an IDE, building the project and download the program to the target board.
   More information about how to compile and program the project can refer to the 

   [Getting Started with MCUXpresso SDK](../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

## Expected Result
------------------------
Input source and LED status table:

 PLU_IN4 | PLU_IN3 | PLU_IN2 | LED_BLUE | LED_GREEN | LED_RED
---------|---------|---------|----------|-----------|---------
 0       | 0       | 0       | off      | off       | off
 0       | 0       | 1       | off      | off       | on
 0       | 1       | 0       | off      | on        | off
 0       | 1       | 1       | on       | off       | off
 1       | 0       | 0       | off      | on        | on
 1       | 0       | 1       | on       | off       | on
 1       | 1       | 0       | on       | on        | off
 1       | 1       | 1       | on       | on        | on

The log below shows the output of the plu combination example in the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PLU combination driver example.

Select the input source.
1. Input source 0
2. Input source 1
3. Input source 2
4. Set all three input sources.
0
Select the input value.
0. Low level.
1. High level.
1
Select the input source.
0. Input source 0
1. Input source 1
2. Input source 2
3. Set all three input sources.
3
Input the three values like 000.
0. Low level.
1. High level.
011
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The LED will change as the description in the table