Overview
========
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters.
The example toggles an output per counter when a match occurs.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55s69 board
- Personal Computer

# Board settings

```
Output signal		Board location
SCT0_OUT2    		P18-11
SCT0_OUT6    		P18-13 
```

# Prepare the Demo
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (P6) on the target board.
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
The log below shows example output in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to use it in 16-bit mode
The example shows both 16-bit counters running and toggling an output periodically
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Probe oscilloscope at P18_11(5HZ) and P18_13(2.5HZ) to see output signal.
