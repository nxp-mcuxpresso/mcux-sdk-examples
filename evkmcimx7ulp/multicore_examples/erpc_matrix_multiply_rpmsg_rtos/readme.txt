Overview
========
The Multicore eRPC Matrix Multiply RTOS project is a simple demonstration program that uses the
MCUXpresso SDK software and the Multicore SDK to show how to implement the Remote Procedure Call
between cores of the multicore system. The primary core (eRPC client) releases the secondary core
(eRPC server) from the reset and then the erpcMatrixMultiply() eRPC call is issued to let the
secondary core to perform the multiplication of two randomly generated matrices. The original
matrices and the result matrix is printed out to the serial console by the primary core. The
matrix multiplication can be issued repeatedly when pressing a SW board button. RPMsg-Lite erpc
transport layer is used in this example application.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- 5V power supply
- Personal Computer

Board settings
==============
The Multicore eRPC Matrix Multiply RTOS project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board jumper settings and
configurations in default state when running this demo.

#### Please note gcc ram debug target exceeds the RAM size so only ram release target and flash targets
are available!

Prepare the Demo
================
1.  Connect 5V power supply to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Open both serial lines provided at J6 USB port.
    (e.g. /dev/ttyUSB0 and /dev/ttyUSB1 on Linux or COM5 and COM6 on Windows)
5.  Flash the image for Cortex-M4 to QSPI using u-boot.
6.  Copy python folder containing the Linux part of the eRPC multicore demo to root file system
7.  Boot Linux
8.  After login, make sure imx_rpmsg_tty kernel module is inserted (lsmod) or insert it (modprobe imx_rpmsg_tty).
9.  In Linux console, browse to the python directory containing example.py
10. Figure out the rpmsg tty device, i.e. /dev/ttyRPMSG101
11. Put the above rpmsg tty device name as a commandline parameter for example.py script like: python example.py /dev/ttyRPMSG101
12. Press any key to generate a random 5x5 matrix and send a multiplication request to Cortex-M4.

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
After the boot process succeeds, the ARM Cortex-M4 terminal displays the following information:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
eRPC Matrix Multiply demo started...

eRPC setup done, waiting for requests...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The log below shows the output of the eRPC Matrix Multiply RTOS demo in the Cortex-A terminal window
after run example.py:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Selected ttyRPMSG Transport

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

Press any key to initiate the next matrix multiplication
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
