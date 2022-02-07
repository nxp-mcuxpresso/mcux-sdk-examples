Overview
========
The sai_edma_tdm example shows how to use sai edma driver with TDM format data:

In this example, one sai instance playbacks the audio data stored in the sdcard using EDMA channel.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- Headphone(OMTP standard) or speaker
- RCA jack
- cs42888 audio board
- sdcard(formatted with FAT32 format).

Board settings
==============
CS42888 audio board             rt1050 evk board
A4                      ->          GND
A5                      ->          SAI1_TX_BCLK(BCLK)
A6                      ->          SAI1_TX_SYNC(SYNC)
A8                      ->          SAI1_TXD(TX)
A9                      ->          3V3
A11                     ->          J22 pin8(RESET- GPIO1, PIN19)
A13                     ->          5v
A16                     ->          1v8
B5                      ->          I2C1_SCL(scl)
B6                      ->          I2C1_SDA(sda)
B12                     ->          SAI1_MCLK(MCLK)

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
4. connect CS42888 audio board to evk board.
5. Decompression the 8_TDM.zip and put 8_TDM.wav into the sdcard(formatted with FAT32 format).
6. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, you can hear

AOUT0 - Front left
AOUT1 - Front right
AOUT2 - Centre
AOUT3 - "tone"
AOUT4 - Back Left
AOUT5 - Back right

and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~
SAI edma TDM example started.

Please insert a SDCARD into board, make sure the sdcard is format to FAT32 format and put the 8_TDM.wav file into the sdcard.

Card inserted.

8_TDM.wav File is available

CS42888 codec Init Done.

Start play 8_TDM.wav file.

SAI TDM EDMA example finished.
 ~~~~~~~~~~~~~~~~~~~
