Overview
========
The SDCARD Interrupt project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continusly. The purpose of this example is to show how to use SDCARD driver and
show how to use interrupt based transfer API in SDHC driver in SDK software to access SD card.
Note: If DATA3 is used as the card detect PIN, please make sure DATA3 is pull down, no matter internal or external, at the same time, make sure the card can pull DATA3 up, then host can detect card through DATA3.And SDHC do not support detect card through CD by host, card can be detected through DATA3 or GPIO.No matter detect card through host or gpio, make sure the pinmux configuration is correct.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K66F board
- Personal Computer

Board settings
==============
The SDCARD Interrupt example is configured to use SDHC0 with PTE0, PTE1, PTE2, PTE3, PTE4, PTE5 pins
and use PTD10 pin as card detection pin.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
SDCARD interrupt example.

Please insert a card into board.

Card inserted.

Card size xxxx bytes

Working condition:

  Voltage : xxxxV

  Timing mode: High Speed

  Freq : xxxx HZ

Read/Write/Erase the card continuously until encounter error......

Write/read one data block......
Compare the read/write content......
The read/write content is consistent.
Write/read multiple data blocks......
Compare the read/write content......
The read/write content is consistent.
Erase multiple data blocks......

Input 'q' to quit read/write/erase process.                
Input other char to read/write/erase data blocks again.


