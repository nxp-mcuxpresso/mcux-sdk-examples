Overview
========
The Power mode switch demo application demonstrates the use of power modes in the KSDK. The demo prints the power mode menu
through the debug console, where the user can set the MCU to a specific power mode. The user can also set the wakeup
source by following the debug console prompts. The purpose of this demo is to show how to switch between different power
 modes, and how to configure a wakeup source and wakeup the MCU from low power modes.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary module during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, the pin current would be activated unexpectedly waste some energy.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume addtional power, had better to disable related pins or disconnect them. 


Toolchain supported
===================
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- i.MX8QX MEK Board
- MCIMX8-8X-BB
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
The example requires connecting CAN pins into CAN network to wakeup the M4.
The driver_examples/flexcan/interrupt_transfer example can provide a CAN wakeup signal:
The board running power_mode_switch example acts as node B. The board used to wakeup should run node A application.

Connect two boards:
- J34-2(CANL) node B, J34-2(CANL) node A
- J34-7(CANH) node B, J34-7(CANH) node A

Note: 
1. The M4 can only enter partial LLS/VLLS(return fail entering LLS/VLLS) when there's peripheral in this M4 Subsystem is running. 
   The debugger also may block the M4 fully enter LLS/VLLS.
2. This demo uses classical CAN function, if using driver_examples/flexcan/interrupt_transfer example as wakeup source, the node A
   should also work in classical CAN mode
3. With the introduction of multiple partition and SRTM, the demo now must run together with Linux kernel. Running this demo in
   single partition mode or before kernel boot up will get wrong result

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board (Refer "Getting Started with MCUXpresso SDK for i.MX 8QuadXPlus.pdf" for debug port information).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When running the demo, the debug console shows the menu to tell how to set MCU
to target power mode.

NOTE: Only input when the demo asks to input, input at other time might make the
debug console overflow and get wrong result.
~~~~~~~~~~~~~~~~~~~~~
Task A is working now

MCU wakeup source 0x80...

####################  Power Mode Switch Task ####################

    Build Time: Jul 25 2018--20:56:49
    Core Clock: 264000000Hz
    Power mode: RUN

Select the desired operation

Press  A for enter: RUN      - Normal RUN mode
Press  B for enter: WAIT     - Wait mode
Press  C for enter: STOP     - Stop mode
Press  D for enter: VLPR     - Very Low Power Run mode
Press  E for enter: VLPW     - Very Low Power Wait mode
Press  F for enter: VLPS     - Very Low Power Stop mode
Press  G for enter: LLS      - Low Leakage Stop mode
Press  H for enter: VLLS     - Very Low Leakage Stop mode

Waiting for power mode select..

~~~~~~~~~~~~~~~~~~~~~
