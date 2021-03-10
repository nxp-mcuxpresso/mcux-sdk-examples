## Overview
-----------
The pin_interrupt example shows how to use PINT to make the pin interrupt events.

When the PINT feature is enabled, initialize the port and the pin connected to SW1 and SW2, configure the port mask with MASK register
and PIN register, which can be used to trigger the PIN's status. At the same time when the SW1 or SW2 is pressed, the the pin interrupt event will
show.

## Functional Description
-------------------------
In this example, The pin_interrupt will output the pin interrupt event into serial terminal. 

When start running this example, main routine will initialize clock, pin mux configuration,
and configure the PINT module to make it work in interrupt way. Users are prompted to 
press the SW1 or SW2, board will output the pattern match event.

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

## Run the Project
------------------------
Run this example by performing the following steps:

1. Connect a micro USB cable between the PC host and the CMSIS DAP port(J3 on the 
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

4. Start the slave board on another board first, then launch the debugger in your IDE to
   begin running this project.

5. Monitor the information on the debug console.

## Expected Result
------------------------
PINT Pin interrupt example

PINT Pin Interrupt events are configured

Press corresponding switches to generate events

2.  This example configures "Pin Interrupt 0" to be invoked when SW1 switch is pressed by the user.

The interrupt callback prints "PINT Pin Interrupt 0 event detected". "Pin Interrupt 1" is
   
is configured to be invoked when SW2 is pressed. The interrupt callback prints "PINT Pin Interrupt 
   
1 event detected".
