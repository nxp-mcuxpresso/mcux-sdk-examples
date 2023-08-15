Overview
========
This example demonstrate how to wake up main device in low power mode with accelerometer trigger event.
The accelerometer can keep working while the main device is in low power mode (or deep sleep mode).
Only when the configured event was captured, the accelerometer will trigger the interrupt to wake up
the main device.
This example uses I2C to configure the accelerometer to work in 800Hz data rate with low noise mode.
And when the tap event is triggered, it will wake up the main device. And 32 samples around the trigger
event will be captured.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2C example -- Accelerometer Event Trigger
Press any key to enter low power mode
Enter deep sleep, tap the board to wake it up.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then tap the board at anywhere, you can see the device is woken up and the 32 samples around the
trigger point were captured.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Woken up!
Interrupt source 0x8.
Pulse trigger source 0xc4.
 0 -- X:   18,  Y:  -72,  Z: 2157
 1 -- X:   41,  Y:  -61,  Z: 2140
 2 -- X:   31,  Y:  -83,  Z: 2144
 3 -- X:   29,  Y:  -83,  Z: 2152
 4 -- X:   33,  Y:  -67,  Z: 2152
 5 -- X:   46,  Y:  -50,  Z: 2147
 6 -- X:   46,  Y:  -39,  Z: 2152
 7 -- X:   32,  Y:  -57,  Z: 2143
 8 -- X:   31,  Y:  -64,  Z: 2147
 9 -- X:   22,  Y:  -73,  Z: 2155
10 -- X:   27,  Y:  -73,  Z: 2165
11 -- X:   35,  Y:  -62,  Z: 2156
12 -- X:   43,  Y:  -49,  Z: 2136
13 -- X:   43,  Y:  -47,  Z: 2142
14 -- X:   41,  Y:  -63,  Z: 2131
15 -- X:   27,  Y:  -77,  Z: 2148
16 -- X:   24,  Y:  -82,  Z: 2154
17 -- X:   36,  Y:  -69,  Z: 2160
18 -- X: -104,  Y:  576,  Z:  -34
19 -- X:  933,  Y: -115,  Z: 3350
20 -- X: -180,  Y: -207,  Z: 2638
21 -- X: -144,  Y:    1,  Z: 1531
22 -- X:  -86,  Y:  141,  Z: 2940
23 -- X:  246,  Y: -233,  Z: 1544
24 -- X:  152,  Y:  166,  Z: 2889
25 -- X:  -42,  Y:   34,  Z: 1820
26 -- X:   36,  Y:   82,  Z: 1678
27 -- X:  187,  Y:  -56,  Z: 1858
28 -- X:  -21,  Y:  -82,  Z: 2670
29 -- X: -179,  Y: -137,  Z: 2252
30 -- X:  -53,  Y: -106,  Z: 1008
31 -- X:   -3,  Y: -121,  Z: 2281
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
