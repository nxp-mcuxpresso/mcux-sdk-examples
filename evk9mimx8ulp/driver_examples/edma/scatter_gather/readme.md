Overview
========
The EDMA memory to memory example is a simple demonstration program that uses the SDK software.
It excuates one shot transfer from source buffer to destination buffer using the SDK EDMA drivers.
The purpose of this example is to show how to use the EDMA and to provide a simple example for
debugging and further development.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 Board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Generate flash.bin with imx-mkimage on host pc
    # git clone <imx-mkimage repo url>
    # cd imx-mkimage
    # cp <m33 program>.bin imx-mkimage/iMX8ULP/
    for single boot type:
    # make SOC=iMX8ULP REV=A1 flash_singleboot_m33 (for A1 sillicon)
    # make SOC=iMX8ULP REV=A0 flash_singleboot_m33 (for A0 sillicon)
    for low power boot type/dual boot type:
    # make SOC=iMX8ULP REV=A1 flash_dual_m33
    # make SOC=iMX8ULP REV=A0 flash_dual_m33

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
EDMA scatter_gather transfer example begin.

Destination Buffer:
0       0       0       0       0       0       0       0

EDMA scatter_gather transfer example finish.

Destination Buffer:
1       2       3       4       5       6       7       8
~~~~~~~~~~~~~~~~~~~~~
