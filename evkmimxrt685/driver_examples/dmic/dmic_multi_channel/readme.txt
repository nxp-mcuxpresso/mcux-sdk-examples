Overview
========
The dmic multi channel example is a simple demonstration program about how to use the DMIC multi channel to gathering audio data with dma.
Audio is converted to samples in the DMIC module.Then, the data is placed into the memory buffer. Last, it is send out through I2S.
In this example, dmic channel 0, 1, 2, 3 will be used to record audio data, the raw audio data will be stored in buffer as:
dmic0, dmic1, dmic2, dmic3, dmic0, dmic1.......
Then another dma channel will convert the raw audio data to the reqirued TDM format,
dmic0, 0, dmic1, 0, dmic2, 0, dmic3, 0......
At last the processed data will be sent out through I2S with TDM format.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 boards
- Personal Computer
- 8-DMIC board
- CS42888 audio board

Board settings
==============
The board enables octal flash (U19) by default. To enable external DMIC (J31), move the following resisters to 2-3 (1-2 by default)
 - R379,R380,R384,R389,R390,R391,R392.

Connect 8-DMIC board to J31.
To make example work, connections needed to be as follows:
CS42888 audio board             rt600 evk REVE board
A4                      ->          GND
A5                      ->          J27 PIN3(BCLK)
A6                      ->          J27 pin2(WS)
A8                      ->          J27 pin1(TX)
A9                      ->          3V3
A11                     ->          J28 pin1(RESET)
A13                     ->          5v
A16                     ->          1v8
B5                      ->          J28 pin10(scl)
B6                      ->          J28 pin9(sda)
B12                     ->          J27 pin6(MCLK)

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert 8dmic board to J31
4.  Connect CS42888 audio board to RT600 EVK per above description.
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

DMIC multi channel example.



Init CS42888 codec



Start play audio data


2. During the DMIC start receive data, you can speak or play song nearby the dmic, then you can hear the multichannel audio data from the CS42888 audio board, AOUT0, AOUT2, AOUT4.
