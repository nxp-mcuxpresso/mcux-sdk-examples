Overview
========
The dmic multi channel example is a simple demonstration program about how to use the DMIC multi channel to gathering audio data with dma.

In this example, dmic channel 0, 1, 2, 3, 4, 5, 6, 7 will be used to record audio data, the raw audio data will be stored:
In buffer 0:
dmic0, dmic1, dmic2, dmic3, dmic0, dmic1.......
In buffer 1:
dmic4, dmic5, dmic6, dmic7, dmic4, dmic5.......

Then another two dma channels will convert the raw audio data to the required TDM format,
One DMA channel responsible for copy buffer 0 to the playback buffer
Another DMA channel responsible for copy buffer 1 to the playback buffer

Finally, the data in the playback buffer will be organized as below:
dmic0, dmic4, dmic1, dmic5, dmic2, dmic6, dmic3, dmic7......

At last the data in the playback buffer will be sent out through I2S with TDM format.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 boards
- Personal Computer
- 8-DMIC board
- CS42448 audio board
- Headphone

Board settings
==============
EVKMIMXRT595:
1. Remove resitor R781 , R782 switch to 2-3 side to disable on board DMIC.
2. Connect 8-DMIC board to J31.
   For the new 8-DMIC DMIC board(SCH-48278 RevA), as the addtitional I2C pin provided by the connector(J1),
   so please make sure the J31 pin1 on EVKMIMXRT595 is align with J1 pin1 on the dmic board.
3. Make sure JS5 & JS6 are installed.

CS42448 Audio board:
Install J5 2-3, J3 2-3, J4 2-3, J2, J13, J14.

To make example work, connections needed to be as follows:
CS42448 audio board             rt500 evk board
J1-2                      ->          JP25 pin1(GND)
J1-23                     ->          JP26 pin4(FC5 BCLK)
J1-16                     ->          JP26 pin3(FC5 WS)
J1-17                     ->          JP26 pin2(FC5 TX)
J1-4                      ->          J29 pin2(3v3)
J1-21                     ->          JP26 pin1(power enable)
J1-10                     ->          J29 pin8(5v)
J1-13                     ->          J36 pin6(I2C2 SCL)
J1-15                     ->          J36 pin8(I2C2 SDA)
J1-1                      ->          JP27 pin2 (MCLK)


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
4.  Connect CS42448 audio board to RT500 EVK per above description.
5.  Connect headphone to the audio board.
6.  Download the program to the target board.
7.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note: As the 8-DMIC board support 8 dmics, so the demo provide several macros to control the specific DMIC's functionality,
#define DEMO_ENABLE_DMIC_0 1
#define DEMO_ENABLE_DMIC_1 1
#define DEMO_ENABLE_DMIC_2 1
#define DEMO_ENABLE_DMIC_3 1

#define DEMO_ENABLE_DMIC_4 1
#define DEMO_ENABLE_DMIC_5 1
#define DEMO_ENABLE_DMIC_6 1
#define DEMO_ENABLE_DMIC_7 1
All the DMIC enabled by default, you can modify the macro to test one DMIC or several DMICS as you like. But please enable at least one DMIC for the demo, otherwise there will be a compile error generate.

1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

DMIC multi channel example.

Init codec

Start play audio data


2. During the DMIC start receive data, you can speak or play song nearby the dmic, then you can hear the multichannel audio data from the CS42448 audio board Line output.
