Overview
========
The dmic i2s hwvad example is a simple demonstration program which intergrated the dmic multi channel/hwvad/i2s playback.

In this example, dmic channel 0, 1, 2, 3, 4, 5, 6, 7 will be used to record audio data and detect voice activity while CPU in deep sleep mode.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK boards
- Personal Computer
- 8CH-DMIC board
- Headphone

Board settings
==============
1. Connect 8-DMIC board to J31.
2. JP44 2-3, JP45 2-3
3. R389 1-2, R390 1-2, R391 1-2, R392 1-2
4. Connect R396 2 to 3.

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
4.  Download the program to the target board.
5.  Insert Headphone to J4 or J50 or J51 or J52.
6.  Launch the debugger in your IDE to begin running the demo.

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

dmic i2s hwvad example.



    1: Record and playback



    2: Enter sleep mode



2. Please press corresponding charactor according to print out menu. If user select '1', then user can hear the sound record by the 8-DMIC from headphone, if user select '2', then the CPU will enter deep sleep mode, it will be woke up if voice activity detected, that is to say user can speak near the 8-DMIC to wake up CPU, once CPU wake up, then you can see the LED array on the 8-DMIC board blinky.

3. The CPU will enter deep sleep mode automatically if user input timeout event occur.
