## Overview
-----------
The pattern_match example shows how to use PINT to make the pattern match events.

When the PINT feature is enabled, initialize the port and the pin connected to SW1 and SW2, configure the port mask with MASK register
and PIN register, which can be used to trigger the PIN's status. At the same time when the SW1 or SW2 is pressed, the the pattern match event will
show.

## Functional Description
-------------------------
In this example, The pattern_match will output the pattern match event into serial terminal. 

When start running this example, main routine will initialize clock, pin mux configuration,
and configure the PINT module to make it work in interrupt way. Users are prompted to 
press the SW1 or SW2, board will output the event.

## Toolchain Supported
---------------------
- IAR embedded Workbench 8.50.5
- Keil MDK 5.31
- GCC ARM Embedded  9.2.1
- MCUXpresso 11.2.0

## Hardware Requirements
------------------------
- Micro USB cable
- LPCXpresso802 board
- Personal Computer

## Board Settings
------------------------

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(CN1 on the 
   LPCXpresso802 board) for receiving debug information.

2. Open a serial terminal in PC(for example PUTTY) with the following settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control

3. Compile and download the program to the target board.
   More information about how to compile and program the project can refer to 

   [Getting Started with MCUXpresso SDK](../../../../../docs/Getting Started with MCUXpresso SDK.pdf).

4. Start the slave board on another board first, then launch the debugger in your IDE to
   begin running this project.

5. Monitor the information on the debug console.

## Expected Result
------------------------
PINT Pattern Match example.

PINT Pattern match events are configured.

Press corresponding switches to generate events.

This example configures "Pin Interrupt 0" to be invoked when ISP key switch is pressed by the user. 

Bit slice 0 is configured as an endpoint in sticky falling edge mode. The interrupt callback prints 

"PINT Pin Interrupt 0 event detected. PatternMatch status =        1". 

"Pin Interrupt 2" is configured to be invoked when rising edge on ISP key, USER key is detected. The 
   
interrupt callback prints "PINT Pin Interrupt 2 event detected. PatternMatch status =     100". Bit slices
  
1 configured to detect sticky rising edge. Bit slice 2 is configured as an endpoint.
