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
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 boards
- Personal Computer
- 8-DMIC board
- IMX-AUD-IO audio board
- RCA to 3.5mm audio line
- Headphone

Board settings
==============
Connect 8-DMIC board to J31.
To make example work, connections needed to be as follows:
IMX-AUD-IO audio board             rt500 evk board
A4                      ->          JP25 pin1(GND)
A5                      ->          JP26 pin4(FC5 BCLK)
A6                      ->          JP26 pin3(FC5 WS)
A8                      ->          JP26 pin2(FC5 TX)
A9                      ->          J29 pin2(3v3)
A11                     ->          JP26 pin1(RESET)
A13                     ->          J29 pin8(5v)
A16                     ->          JP26 pin6(1v8)
B5                      ->          J36 pin6(I2C2 SCL)
B6                      ->          J36 pin8(I2C2 SDA)
B12                     ->          JP27 pin2 (MCLK)

Make sure the JS5 and JS6 is installed and J40 is connected(For I2C pins external pull up).

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert 8dmic board to J31
4.  Connect IMX-AUD-IO audio board to RT500 EVK per above description.
5.  Connect RCA to 3.5mm audio cable to audio board and connect headphone to the RCA to 3.5mm audio cacble.
6.  Download the program to the target board.
7.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

DMIC multi channel example.



Init CS42888 codec



Start play audio data


2. During the DMIC start receive data, you can speak or play song nearby the dmic, then you can hear the multichannel audio data from the IMX-AUD-IO audio board.
If you are using IMX-AUD-IO REVA, the audio will come out from AOUT0, AOUT2, AOUT4.
If you are using IMX-AUD-IO REVB1, the audio will come out from AUDIO-OUT-C, AUDIO-OUT-RL, AUDIO-OUT-FL.
