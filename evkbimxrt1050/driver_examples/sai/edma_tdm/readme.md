Overview
========
The sai_edma_tdm example shows how to use sai edma driver with TDM format data:

In this example, one sai instance playbacks the audio data stored in the sdcard using EDMA channel.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- Headphone(OMTP standard) or speaker
- cs42448 audio board
- sdcard(formatted with FAT32 format).

Board settings
==============
CS42448 Audio board:
Install J5 2-3, J3 2-3, J4 2-3, J2, J13, J14.

To make example work, connections needed to be as follows:
CS42448 audio board             rt1050 evk board
J1-2                      ->          GND
J1-23                     ->          SAI1_TX_BCLK(BCLK)
J1-16                     ->          SAI1_TX_SYNC(SYNC)
J1-17                     ->          SAI1_TXD(TX)
J1-4                      ->          3V3
J1-21                     ->          J22 pin8(power enable)
J1-10                     ->          5v
J1-13                     ->          I2C1_SCL(scl)
J1-15                     ->          I2C1_SDA(sda)
J1-1                      ->          SAI1_MCLK(MCLK)
J1-7                      ->          J22 pin7(codec reset)

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. connect CS42448 audio board to evk board.
5. Decompression the 8_TDM.zip and put 8_TDM.wav into the sdcard(formatted with FAT32 format).
6. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, you can hear

Line 1&2 output: "Front Left", "Front Right"
Line 3&4 output: "Centre", tone
Line 5&6 output: "Back Left", "Back Right"
Line 7&8 output: "Auxiliary Left", "Auxiliary Right"

and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~
SAI edma TDM example started.

Please insert a SDCARD into board, make sure the sdcard is format to FAT32 format and put the 8_TDM.wav file into the sdcard.

Card inserted.

8_TDM.wav File is available

Codec Init Done.

Start play 8_TDM.wav file.

SAI TDM EDMA example finished.
 ~~~~~~~~~~~~~~~~~~~
