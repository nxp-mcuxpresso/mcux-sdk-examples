Overview
========
The GLIKEY Example project is a demonstration program that uses the MCUX SDK software to set up GLIKEY IP peripherial.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the USB port on the board
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
The log below shows the output of the GLIKEY driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GLIKEY Peripheral Driver Example

*Success* GLIKEY is not locked

*Success* GLIKEY Sync Reset done

*Success* GLIKEY Start enable done

*Success* GLIKEY Continue enable Step 1 pass

*Success* GLIKEY Continue enable Step 2 pass

*Success* GLIKEY Continue enable Step 3 pass

*Success* GLIKEY Continue enable Step Enable pass

SUCCESS: Register on index 2 can be now written!

*Success* GLIKEY End of operation done

End of example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

