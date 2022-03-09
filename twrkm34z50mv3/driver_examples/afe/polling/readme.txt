Overview
========
The AFE Example project is a demonstration program that uses the KSDK software.
In this example, the AFE module samples the voltage difference of the EXT_SD_ADP0 and EXT_SD_ADM0 pins.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini USB cable
- TWR-KM34Z50MV3 board
- Personal Computer

Board settings
==============
This project samples the voltage difference of the EXT_SD_ADP0(J17 pin 2) and EXT_SD_ADM0(J17 pin 16) pins. 
For EXT_SD_ADM0, connect it to GND (short J17 15-16). For EXT_SD_ADP0, there are three methods:
1.Connect DC power supply to EXT_SD_ADP0(J17 pin 2), when changing power supply voltage, the AFE sample result changes accordingly.
2.Connect potentiometer voltage (J18 pin 2) to EXT_SD_ADP0(J17 pin 2), when rolling the potentiometer, the AFE sample result changes accordingly.
3.Connect USB port J16 to PC, then voltage waveform is generated on J17 pin 1, short J17 pin 1-2, the AFE samples the waveform.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
These instructions are displayed/shown on the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AFE polling example.

For example, roll the potentiometer to see value changes on terminal screen. If value of potentiometer output lager than 500mV,
result will return maximum of 24 bit (8388607).
The printed log is displayed in the terminal window as shown below.

AFE Polling example.
Press any key to trigger AFE conversion
AFE value  = 27744
Press any key to trigger AFE conversion
AFE value  = 10608
Press any key to trigger AFE conversion
AFE value  = 5814
Press any key to trigger AFE conversion
AFE value  = 73644
Press any key to trigger AFE conversion
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
