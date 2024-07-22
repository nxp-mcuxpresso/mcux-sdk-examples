Overview
========
This example demonstrates usage of eRPC between PC and board using I2C transport layer.
Board acts like a server and the PC as client. The libusbsio Python module is used for 
accessing the NXP libusbsio APIs and thus allowing to exercise SPI, I2C bus and GPIO pins 
from Python application (over USBSIO interface of NXP LPCLink2 and MCUlink Pro devices).
Note, that when more than one board with LPCLink2/MCUlink devices
is connected to the PC the Python application is not able to automatically detect which one to use to establish the communication. Thus, please
ensure only one board with running eRPC server application is connected to the PC when starting the Python application on the PC side 
or use the -d command line argument of the Python application to specify the targetted LPCLink2/MCUlink device index. 

When client starts, it generates two random
matrixes and sends them to server. Server then performs matrix multiplication and sends
result data back to client. Result matrix is then printed on the PC side.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

PC Side Setup (Python)
1. Make sure you have Python installed on your PC
2. Install serial module by executing following command in command line: "python -m pip install pyserial"
3. Install eRPC module to Python by executing setup.py located in: middleware\multicore\erpc\erpc_python - "python setup.py install"
4. Install libusbsio module to Python by executing following command in command line: "python -m pip install libusbsio"

Usage of run_i2c.py
usage: run_i2c.py [-h] [-b BD] [-d DEVIDX]

eRPC Matrix Multiply example

optional arguments:
  -h, --help                    show this help message and exit
  -b BD, --bd BD                Baud rate (default value is 100000)
  -d DEVIDX, --devidx DEVIDX    LIBUSBSIO device index (default value is 0 - only one board connected to the host PC)

Example:
To run PC side as a client with a board connected as a server:
"run_i2c.py --bd 100000"

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
Populate R153, R154, R155, R156, R159, R160
MCU Link firmware version 2.263 and above needs to be used
Use libusbsio Python module version 2.1.11 (https://pypi.org/project/libusbsio/2.1.11)

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCULink USB port (J17) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
When the target device application is running execute Python run_i2c.py from boards\frdmmcxn236\multiprocessor_examples\erpc_common\erpc_matrix_multiply folder:
python run_i2c.py --bd 100000

When the demo runs successfully, the log below shows the output of the Client eRPC Matrix Multiply demo
in the Python Shell window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
