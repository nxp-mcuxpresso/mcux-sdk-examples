Overview
========
The pdm multi channel standalone transfer example shows how to use pdm driver with edma to receive multi DMIC data into standalone buffer:

In this example, pdm will trigger edma to transfer data when one PDM channel watermark value is reached, the DMIC data format,
 ----------------------------------------------------------------------------------------------------------------------
 |CHANNEL0 | CHANNEL0 | CHANNEL0 | .... | CHANNEL1 | CHANNEL 1 | CHANNEL1 |....| CHANNEL2 | CHANNEL 2 | CHANNEL2 |....|
 ----------------------------------------------------------------------------------------------------------------------

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output demo in the terminal window:
Note: only PDM channel 4&5 has valid data according to the EVK default configurations.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM channel 3
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
PDM channel 4
fffffe00 ffff8900 e00 19100 28900 20a00 14c00 4f00 fffd2500 fffb1800 fffac500 fffbdb00 fffe3000 ffff5600 fffde800 fffc6d00 fffd0200 fffd3300 fffc3c00 fffade00 fffa7500 fffa9e00 fffaf600 fffc1600 fffe0100 ffff7d00 ffff4b00 fffd5c00 fffb4000 fffbfc00 fffc9b00 fffbf100 fffcae00 fffef400 27d00 39800 22e00 1f000 24200 3a400 37200 13c00 1af00 38400 4e700 4f800 40000 4e500 75000 7cb00 5d600 39600 35b00 4da00 58100 5f300 71e00 84300 9f300 ac500 ae900 c4400 c6200 9fd00 6c000 51300 56b00 5eb00 5bb00 59500 64800 76a00 90000 88a00 62f00 5c500 54600 38d00 1b500 c500 2d300 5a000 74600 73800 52200 42f00 33b00 28600 2bc00 33f00 48b00 40500 23700 18200 4e300 9bd00 b1e00 8c300 57f00 59b00 83f00 99900 7da00 5e800 46c00 3c600 57100 62600 58400 30100 13900 22e00 35400 4ea00 49200 27f00 13500 22c00 54b00 7c300 78d00 5ab00 5fe00 84f00 b1000 c7c00 b1a00 a2700
PDM channel 5
fffff200 ffff9100 2b00 1f800 22400 29a00 20900 7f00 fffdea00 fffadf00 fffb5a00 fffccc00 fffdec00 fffeea00 fffd4d00 fffc1b00 fffcc100 fffdb200 fffcbb00 fffaf400 fffa9d00 fffa4000 fffb2b00 fffc5600 fffd5500 fffec800 fffe9800 fffcfe00 fffb8c00 fffc2c00 fffcac00 fffb5c00 fffc0c00 ffff0f00 27a00 42600 2c200 1a300 28800 48600 41300 27300 34f00 42a00 50200 50a00 45c00 59600 7eb00 87900 67000 48b00 45400 51e00 5e300 6c900 79400 89b00 a6b00 b5b00 bf000 cc200 c6b00 a6100 7b100 5e100 59700 6ba00 6ec00 61c00 6b600 84700 98d00 8a800 66d00 54c00 47300 34400 13500 2c00 16000 44300 6ba00 65000 4e300 2db00 1b200 1cf00 1e500 29300 40500 35100 d900 ae00 39200 87d00 a5300 82100 4f800 48f00 76700 89900 75b00 54600 35100 35400 47600 50500 49b00 1e600 8000 14c00 30900 47c00 33a00 1b400 b400 1a300 50800 70700 66b00 53200 59700 7fb00 b3b00 c7100 b2f00 a7a00
PDM channel 6
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
PDM channel 3
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
PDM channel 4
b3900 bd000 a4800 9cb00 9f200 90100 87200 7b100 79300 5d700 35200 38b00 44500 47b00 45000 56c00 6dc00 70700 74d00 80a00 89600 81300 72b00 5a800 3f300 2a500 2f400 54800 68a00 5f700 5ac00 44b00 22400 17f00 2100 fffdff00 fffb9500 fff9af00 fff83e00 fff89200 fffaaf00 fffb4c00 fff9e400 fff64c00 fff45500 fff4cf00 fff55300 fff48800 fff42300 fff5a300 fff88200 fffb2800 fffa9700 fffa0000 fffa4a00 fffa4600 fffa1f00 fff97800 fff88300 fff82800 fff8c900 fff96000 fff8e100 fff86b00 fff8d500 fff8b600 fff96a00 fff9dc00 fff89c00 fff6e600 fff5ab00 fff55700 fff62100 fff82500 fff99900 fff9cb00 fff9ef00 fffafb00 fffbd400 fffad200 fff8e300 fff6ad00 fff59200 fff71a00 fff84d00 fff8bf00 fff8a300 fff77e00 fff8ad00 fffad700 fffabb00 fff8d200 fff5bf00 fff2e100 fff23900 fff1de00 fff23900 fff41400 fff4c300 fff4bd00 fff4e600 fff51b00 fff44200 fff2f700 fff2ef00 fff4ae00 fff74000 fff7d200 fff79e00 fff7cd00 fff86f00 fffa0500 fffb7b00 fffd4000 fffc5f00 fffa9300 fffaa700 fffb4a00 fffdd100 ffff5900 fffeac00 fffedf00 cd00 2fc00 27700 7c00 ffff2e00 fffe8900
PDM channel 5
b2200 bd000 a1000 8e700 98900 89f00 82200 76200 69d00 51f00 28d00 1fd00 2c200 35400 37800 48800 5cf00 6c700 6db00 75200 83c00 79700 66000 50d00 39200 25400 2d900 4cf00 61400 5a300 51500 37000 16400 12800 1000 fffe0a00 fffb7c00 fff95200 fff79400 fff7d500 fffa6100 fffb6300 fff9f600 fff5ef00 fff38200 fff46400 fff4fc00 fff4bc00 fff43000 fff51b00 fff86400 fffaa300 fffa2700 fff93900 fff94600 fff99000 fff93200 fff86900 fff73400 fff6d000 fff7de00 fff88200 fff86e00 fff7f500 fff87300 fff8dd00 fff89300 fff95100 fff8a100 fff6de00 fff4e700 fff47d00 fff61f00 fff7d800 fff98f00 fff98500 fff9aa00 fffad800 fffc2300 fffb8f00 fff8c800 fff6b800 fff57100 fff6f400 fff8ed00 fff8b500 fff85b00 fff7da00 fff8e100 fffad700 fffac800 fff81b00 fff4db00 fff2be00 fff1c400 fff17800 fff1c900 fff34b00 fff44200 fff41900 fff41f00 fff42300 fff34c00 fff1ba00 fff20c00 fff4d200 fff6bc00 fff6d800 fff69f00 fff6a600 fff7c700 fff9ad00 fffaa100 fffc0b00 fffb5100 fff97500 fff97100 fff9ce00 fffc9c00 fffe5800 fffdec00 fffe0a00 0 1ea00 11300 ffff2200 fffd6d00 fffd9000
PDM channel 6
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
PDM edma multi channel standalone transfer example finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

