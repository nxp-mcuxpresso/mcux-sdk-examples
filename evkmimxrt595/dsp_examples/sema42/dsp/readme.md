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
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
The DSP images are built into CM33 image with default project configuration.
To build the CM33 image, the DSP images dsp_sram_release.bin, dsp_tcm_release.bin should be built firstly.
In addition, to build CM33 image with MCUXpresso IDE, the DSP images should be copied to
the MCUXpresso IDE workspace folder, like C:\Users\<user_name>\Documents\MCUXpressoIDE_11.0.1_2563\workspace\fusionf1.

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
This example run both M33 and DSP at the same time! 

When the demo runs successfully, the log would be seen on the M33's terminal like below
and the RED LED(D19) will be turned on/off according to the log.

~~~~~~~~~~~~~~~~~~~~~
Sema42 example!

Press any key to unlock semaphore and DSP core will turn off the LED
~~~~~~~~~~~~~~~~~~~~~

When press any key in the terminal, the terminal tells

~~~~~~~~~~~~~~~~~~~~~
Now the LED should be turned off

Sema42 example succeed!
~~~~~~~~~~~~~~~~~~~~~
