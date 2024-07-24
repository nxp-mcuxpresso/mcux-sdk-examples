Overview
========

The dsp_sema42 demo application demonstrates starting DSP core with DSP image.

In this example:
1. Firstly, CM33 core turn on LED  and lock a sema gate then boot up DSP core wake up.
2. DSP core must be wait until CM33 core unlocks this sema gate to lock it.
3. After user press any key in terminal window, the sema gate will be unlocked by CM33 core,
then DSP core will lock it and turn off the LED

If the board does not have LED to show the status, then DSP core will send a flag
to CM33 core when DSP core has locked the sema gate. CM33 core outputs the success log
after it received the flag.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- Xtensa C Compiler  14.01

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============


Prepare the Demo
================
The DSP images are built into CM33 image with default project configuration.
To build the CM33 image, the DSP images dsp_reset_release.bin, dsp_text_release.bin, dsp_data_release.bin should be built firstly.

1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
This example run both M33 and DSP at the same time! 

When the demo runs successfully, the log would be seen on the M33's terminal like below.

~~~~~~~~~~~~~~~~~~~~~
Sema42 example!
Press any key to unlock semaphore and DSP core will lock it

Wait for DSP core lock the semaphore

Sema42 example succeed!
~~~~~~~~~~~~~~~~~~~~~
