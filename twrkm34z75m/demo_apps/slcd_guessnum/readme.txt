Overview
========
The SLCD number guess demo application demonstrates the use of the SLCD peripheral and driver.

The SLCD number guess demo has two demo displays:

1. The first demo is a basic SLCD test, where all numbers and icons are shown one by one on the screen.

2. The second demo is the "Number Guess" game. This demo asks the user to input a number. The demo then provides
clues to the user to help the user guess the correct number.

"Great, xxx you has GOT it!" will be shown on the console, and the victory will be displayed on the LCD screen when the
user successfully guesses the correct number.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini USB cable
- TWR-KM34Z75M board
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
1. Basic Testing
The demo performs basic testing on the SLCD first. All numbers and icons are displayed, one-by-one, on the screen.

2. Guess the number game

Next, the demo asks the user to play a number guessing game.

The user can input a number (0-9999) in the console and the number is displayed on the SLCD screen.
In the example below, if the number is 6396, it took the user four guesses before choosing the correct number.
~~~~~~~~~~~~

---------- Start basic SLCD test -------------
-------------- SLCD Guess Num Game --------------
The number input and final number are shown on the SLCD.
Check SLCD for these numbers.
Let's play:
Please Guess the number I want(0 - 9999), Press 'enter' to end:
~~~~~~~~~~~~

The input number 5000 is smaller than what I want. Guess again!
Guess the number I want(0 - 9999), Press 'enter' to end: 7500
The input number 7500 is bigger than what I want. Guess again!
Guess the number I want(0 - 9999), Press 'enter' to end: 6125
The input number 6125 is smaller than what I want. Guess again!
Guess the number I want(0 - 9999), Press 'enter' to end: 6396
Great, 6396, you have GOT it!
Play again? Input 'Y' or 'N'.
N
~~~~~~~~~~~~~
