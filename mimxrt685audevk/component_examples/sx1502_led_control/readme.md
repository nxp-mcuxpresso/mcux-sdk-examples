Overview
========
The sx1502_led_control Demo application demonstrates to control Leds of external DMIC board by commands.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.
Besides, the demo does not support semihosting mode. The external_dmic_led_control demo is based on shell, debug console and serial manager. When semihosting is used, debug console and serial manager are bypassed. So the sx1502_led_control demo cannot work with semihosting.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK boards
- External 8-DMIC board
- Personal Computer

Board settings
==============
Connect 8-DMIC board to J31.
Connect R396 2 to 3.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~
Configure SX1502 IO expander driver.
The main function of the demo is to control the led on external dmic board by using the shell.Please enter "help" to get the help information firstly.

Format of input command:
To control single LED, Turn on LED by using command "led index on". And turn off LED by using command "led index off".
Such as:
    led 1 on  : LED1 on
    led 1 off : LED1 off
To control multiple LED, Turn on using command "pattern index on". And turn off by using command "pattern index off".When the pattern is off, the LEDs return to the initial state.
Such as:
    pattern 1 on : LED1-LED6 on, LED7 off
    pattern 2 on : LED1/LED3/LED4/LED6 on, LED2/LED5/LED7 off
    pattern 3 on : LED2/LED5/LED7 on, LED1/LED3/LED4/LED6 off
    pattern 4 on : LED1/LED3/LED5/LED7 on, LED2/LED4/LED6 off
    pattern index off : All LEDs return to the initial state.

Copyright 2020 NXP

SHELL>> help

"help": Lists all the registered commands

"exit": Exit program

"led arg1 arg2":
 Usage:
    arg1: 1|2|3|4...            Led index
    arg2: on|off                Led status

"pattern arg1 arg2":
 Usage:
    arg1: 1|2|3|4...            Pattern index
    arg2: on|off                Pattern status

SHELL>> led 1 on
SHELL>> led 1 off
SHELL>> led 2 on
SHELL>> led 2 off
......
SHELL>> led 8 on
LED index is wrong!
SHELL>> pattern 1 on
SHELL>> pattern 1 off
SHELL>> pattern 2 on
SHELL>> pattern 2 off
......
SHELL>> pattern 5 on
Pattern index is wrong!
SHELL>>
~~~~~~~~~~~~~~~~~~~~~
