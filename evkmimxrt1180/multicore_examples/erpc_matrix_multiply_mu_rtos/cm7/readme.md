Overview
========
The Multicore eRPC Matrix Multiply RTOS project is a simple demonstration program that uses the
MCUXpresso SDK software and the Multicore SDK to show how to implement the Remote Procedure Call
between cores of the multicore system. The primary core (eRPC client) releases the secondary core
(eRPC server) from the reset and then the erpcMatrixMultiply() eRPC call is issued to let the
secondary core to perform the multiplication of two randomly generated matrices. The original
matrices and the result matrix is printed out to the serial console by the primary core. The
matrix multiplication can be issued repeatedly when pressing a SW board button. MU (Messaging Unit)
erpc transport layer is used in this example application.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
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
The log below shows the output of the eRPC Matrix Multiply RTOS demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Primary core started
Copy CORE1 image to address: 0x303C0000, size: 15548

Matrix #1
  21   33   37   37    9
  23   45   43    0   32
  38   44    8   15   36
  18   18   38   44   16
  22   23    0   38    7

Matrix #2
  11   23   27   45   11
   7   19   23   24    6
  32   26   49   43   16
  22   48   36   34   41
  27   20   32   31   11

eRPC request is sent to the server

Result matrix
2703 4028 4759 4865 2637
2808 3142 4787 4956 1563
2284 3358 4122 4736 1821
2940 4176 4858 4868 2894
1428 2907 2715 3051 2015

Press the SW8 button to initiate the next matrix multiplication
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy CORE1 image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.

Note:
MIMXRT1180-EVK GPIO_AON_04 pin is shared between SW8 button and headphone HP_DET_B pin. Once a headphone is inserted into
J101 Audio Jack connector the SW8 button functionality is affected, causing eRPC request are triggered repeatedly without
the button being pressed. Please remove the headphone in this case.
