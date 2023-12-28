Overview
========
The lpuart_edma Example project is to demonstrate usage of the KSDK lpuart driver.
In the example, you can send characters to the console back and they will be printed out onto console
 in a group of 8 characters.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No need special setting.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download flash.bin
    for single boot type:
    a. Download to emmc(for single boot type) with uuu
    # uuu -b emmc imx-boot-imx8ulpevk-sd.bin-flash_singleboot_m33 flash.bin (Need download the file imx-boot-imx8ulpevk-sd.bin-flash_singleboot_m33 from linux bsp)
    for low power boot type/dual boot type:
    a. Download to flexspi nor flash of m33 with uboot console(pls refer to the doc Getting Started with MCUXpresso SDK for EVK-MIMX8ULP and EVK9-MIMX8ULP.pdf)
    b. Download to flexspi nor flash of m33(for low power boot type/dual boot type) with jlink(pls refer to the doc Getting Started with MCUXpresso SDK for EVK-MIMX8ULP and EVK9-MIMX8ULP.pdf)
    c. Download to flexspi nor flash of m33 with uuu(write a uuu script to write flash.bin to flexspi nor flash of m33, pls refer to uuu manual)

5.  Power Off board and Power on board
6.  Note:
    Pls not running the demo that use TRDC with debugger(jlink or others)

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
LPUART EDMA example
Send back received data
Echo every 8 characters
~~~~~~~~~~~~~~~~~~~~~
