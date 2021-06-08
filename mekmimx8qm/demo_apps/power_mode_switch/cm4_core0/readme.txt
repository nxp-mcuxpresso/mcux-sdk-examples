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
- i.MX8QM MEK Board
- MCIMX8-8X-BB
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
The example requires connecting CAN pins into CAN network to wakeup the M4 core1.
The driver_examples/flexcan/interrupt_transfer example can provide a CAN wakeup signal:
The board running power_mode_switch example acts as node B. The board used to wakeup should run node A application.

Connect two boards:
- J34-2(CANL) node B, J34-2(CANL) node A
- J34-7(CANH) node B, J34-7(CANH) node A

Note: 
1. The M4s can only enter partial LLS/VLLS(return fail entering LLS/VLLS) when there's peripheral in this M4 Subsystem is running. 
   The debugger also may block the M4s fully enter LLS/VLLS.
2. This demo uses classical CAN function, if using driver_examples/flexcan/interrupt_transfer example as wakeup source, the node A should also work in classical CAN mode, and baudrate is 500K!
   Add "flexcanConfig.baudRate = 500000U;" after "FLEXCAN_GetDefaultConfig(&flexcanConfig);" in flexcan_interrupt_transfer.c.
3. The demo for M4 core1 provides I2C service for Cortex-A core, if Cortex-A core not boot, the "Linkup" task will execute every APP_LINKUP_TIMER_PERIOD_MS define in app_srtm.h.
   To evaluate full feature provided by the demo, please run M4 core0, M4 core1 with Linux and keep M4s and A cores run in different partitions. 
   Run this demo using imx-mkimage with -flags 0x00200000, such as flash_linux_m4, flash_regression_linux_m41, flash_linux_m4_ddr, flash_linux_m4_xip .etc.
   
Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board (Refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for debug port information).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    
    - No flow control
4.  Download the program to the target board (Please refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for how to run different targets).
5.  Power on the board or press reset button to start the example.

Running the demo
================
When running the demo, the debug console shows the menu to tell how to set MCU
to target power mode.

NOTE: Only input when the demo asks to input, input at other time might make the
debug console overflow and get wrong result.
~~~~~~~~~~~~~~~~~~~~~
Task 1 is working now

MCU wakeup source 0x80...

####################  Power Mode Switch Task ####################

    Build Time: Jul  4 2018--17:41:45
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
Press  R for using WDOG trigger M4 partition reset.

Waiting for power mode select..

~~~~~~~~~~~~~~~~~~~~~
For M4 core0, the demo also provides RPMSG pingpong demo to shows how to implement the inter-core
communicaton between Cortex-M4 core and Cortex-A core using rpmsg-lite library. 
For M4 core1, the demo also provides I2C service to work with Linux through SRTM services.
Linux will utilize SRTM APIs to operate on the M4 I2C bus. The commands are transferred
via RPMSG to M core, then based on service protocols, M core will handle the
configuration to M4 I2C bus. Linux side operates on the CS42888 and PCA6416 CAN I/O Expander
I2C bus through this I2C service.

To evaluate the SRTM I2C service:
1. Insert the Audio IO Card into Audio Slot1(J20) on base board.
2. Connect headphone to a A/V RCA to 3.5mm cable. 
   Connect headphone RCA cable to Audio IO Card AOUT 0, AOUT 1. 
3. Boot Linux kernel with M4 specific dtb.
Run the following commands in Uboot console:
~~~~~~~~
setenv fdt_file 'fsl-imx8qm-mek-rpmsg.dtb'
save
run bootcmd
~~~~~~~~
4. After Kernel boot done, use the following command in kernel console to check cs42888-audio card index:
~~~~~~~~
   aplay -l
~~~~~~~~
5. Use the following command in Linux kernel console to test cs42888 audio playback:
~~~~~~~~
   aplay -Dplughw:2 /unit_tests/ASRC/audio8k16S.wav
~~~~~~~~
6. The SRTM support re-connect if one side reset/reboot(M4 and A core need run in different partition, for partition reset info please refer System Controller Firmware Porting Guide).
In M4 console input "R" will reset M4. In Linux kernel console input "reboot" will reset A core.
