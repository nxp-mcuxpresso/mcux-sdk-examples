Overview
========

The dsp_mu_polling demo application demonstrates starting DSP core with DSP image.

In this example:
1. M33 core send message to DSP in polling mode via MU module.
2. DSP send message back to M33 core in polling mode.
3. M33 core receive message sent from DSP in polling mode.


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
This example shows two cores communicating using MU. Both M33 and DSP core run at the same time!

When the demo runs successfully, the log would be seen on the M33's debug terminal like as below:

~~~~~~~~~~~~~~~~~~~~~
MU example polling!
Send: 0. Receive 0
Send: 1. Receive 1
Send: 2. Receive 2
Send: 3. Receive 3
Send: 4. Receive 4
Send: 5. Receive 5
Send: 6. Receive 6
Send: 7. Receive 7
Send: 8. Receive 8
Send: 9. Receive 9
Send: 10. Receive 10
Send: 11. Receive 11
Send: 12. Receive 12
Send: 13. Receive 13
Send: 14. Receive 14
Send: 15. Receive 15
Send: 16. Receive 16
Send: 17. Receive 17
Send: 18. Receive 18
Send: 19. Receive 19
Send: 20. Receive 20
Send: 21. Receive 21
Send: 22. Receive 22
Send: 23. Receive 23
Send: 24. Receive 24
Send: 25. Receive 25
Send: 26. Receive 26
Send: 27. Receive 27
Send: 28. Receive 28
Send: 29. Receive 29
Send: 30. Receive 30
Send: 31. Receive 31
MU example run succeed!
~~~~~~~~~~~~~~~~~~~~~
