Overview
========
The Low Power display demo application demonstrates how to use CM33 core to
do a tripple buffer pan display with total i.MX8ULP SoC power around 90mW.
In this demo, the display controller, MIPI DSI host is controlled by CM33
instead of Application Domain (A35 cores). The frame buffers are put into the external
PSRAM through FlexSPI interfaces, this can keep the DDR in retention mode to save power.
The display content refresh rate is limited to one fps, RTD domain will enter
sleep mode to save power after updated the display buffer address every seconds.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- RK055MHD091 MIPI panel
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Note! ####
1.  This case does not support ddr and flash target. 
2.  This case runs together with Linux.

Prepare the Demo
================
1.  Connect the RK055MHD091 MIPI panel onto EVK board.
2.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
3.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Turn on the power switch on your board to begin running the demo.

Running the demo
================
When running the demo, the debug console shows the menu to command the CM33 to start the low power display. Before you input the command to run the low power display, please put the Linux into suspend (APD domain into Power Down mode) by command in the APD domain console:
    $echo mem > /sys/power/state
After Linux goes into suspend mode, please input the 'Z' in RTD console to start low power display. The display panel will show fullscreen color, the color switch between RED/GREEN/BLUE every second.

~~~~~~~~~~~~~~~~~~~~~
AD entered Active mode
Start SRTM communication
Task 1 is working now

####################  Power Mode Switch Task ####################

    Build Time: May 11 2023--16:54:05
    Core Clock: 160000000Hz
    Boot Type: Single Boot Type

Select the desired operation

Press  Z for low power display.
Press  M for switch Voltage Drive Mode between OD/ND/UD.

Waiting for power mode select..
~~~~~~~~~~~~~~~~~~~~~
