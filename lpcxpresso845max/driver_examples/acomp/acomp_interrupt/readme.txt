## Overview
-----------
The LPC_ACOMP Interrupt Example shows how to use interrupt with ACOMP driver.

In this example, user should indicate an input channel to capture a voltage signal (can be controlled by user) as the 
ACOMP's negative channel input. On the postive side, the internal voltage ladder is used to generate the fixed voltage about
half value of reference voltage.

When running the project, change the input voltage of user-defined channel, then the comparator's output would change
between logic one and zero when the user-defined channel's voltage crosses the internal ladder's output. The change of
comparator's output would generate the falling and rising edge events with their interrupts enabled. When any ACOMP 
interrupt happens, the ACOMP's ISR would turn on/off the LED light.

## Functional Description
-------------------------
In this example, ACOMP will compare the input voltage of user-defoned channel and internal voltage ladder's output, then ACOMP 
will use polling mode to check the result of compare and turn on/off LED.

1. Once the project starts, main routine will initialize clock, pin mux configuration, LED configuration
   and configure the ACOMP module to make it work.

2. Configure ACOMP negative and postive input channels and if use internal voltage ladder, enable and configure internal
   voltage ladder.
   
3. Enable the ACOMP both-edges interrupts and enable the corresponding NVIC.

4. Use interrupt method to check whether the ACOMP output is logic one in ISR when the ACOMP output changes, if yes, turn on the LED,
   otherwise, turn off the LED. 

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
J6-1 is ACOMP negative input pin, which can sample external voltage.

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

4. Start the slave board on another board first, then launch the debugger in your IDE to
   begin running this project.

5. Monitor the information on the debug console.

## Expected Result
------------------------
If the ACOMP input pin(J6-1) connects the GND, LED(D3) will be turned on.
If the ACOMP input pin(J6-1) connects the 3.3V, LED(D3) will be turned off.

