Overview
========
The sai_edma_transfer example shows how to use sai driver with EDMA:

In this example, one sai instance playbacks the audio data stored in flash/SRAM using EDMA channel.

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
- Headphone

Board settings
==============

#### Please note this application can't support running with Linux BSP! ####

Note
~~~~~~~~~~~~~~
Because of the hardware design issue, you can not restart this demo/example by press the Reset Button(SW3):
The I2S0_RXD0 signal is connected to PTA2 of i.MX 8ULP on EVK Board. This pin is also used as BOO_CFG2 pin
when the i.MX SoC reboot. The I2S0_RXD0 signal is driven by on-board WM8960 Codec IC, when user press Reset
Button. This may cause M4 Core enter Boot ROM's "Infinit-Loop" debug mode and only re-powerup the board can
let the M4 core exit such debug mode. 

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
    a. Pls not running the demo that use TRDC with debugger(jlink or others).
    b. Pls use low power boot type when running the demo failed in single boot type.

Running the demo
================
Note: This demo outputs 215HZ sine wave audio signal.
When the demo runs successfully, you can hear the tone and the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~
SAI example started!
SAI EDMA example finished!
 ~~~~~~~~~~~~~~~~~~~
