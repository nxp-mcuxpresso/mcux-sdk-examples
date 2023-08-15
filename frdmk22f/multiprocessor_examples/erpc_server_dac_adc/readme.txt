Overview
========
This example demonstrates usage of eRPC between PC and board using UART transport layer.
Board acts like a server and the PC as client. When server starts, it waits for
data being send from client over UART. Server then performs action (DAC/ADC conversion, light LEDs,
read data from magnetometer senzor) and sends result data back to client (or lights LED).

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

Running with i.MX7D Sabre SD board
- First, download L4.1.15_2.0.0_iMX6UL7D board support package from nxp.com
- Then uncompress it and locate file fsl-image-validation-imx-imx7dsabresd.sdcard
- Insert an SD card and run: sudo dd if=fsl-image-validation-imx-imx7dsabresd.sdcard
of=/dev/mmcblk0 bs=1M 
- Flush disk IO operations, run command: sync
- Download enum34, erpc and pyserial packages from pypi, uncompress them and copy to
/home/root, respective links are:
		- enum34: https://pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz#md5=5f13a0841a61f7fc295c514490d120d0
		- erpc: https://pypi.python.org/packages/fd/0d/64425e9d5258fa5ea9cef7de879e3ae8be6c0d50a7e215ba791d0a32d77e/erpc--1.4.0.tar.gz#md5=1387082dab3af4067fdfe01c98b09dee
		- pyserial: https://pypi.python.org/packages/df/c9/d9da7fafaf2a2b323d20eee050503ab08237c16b0119c7bbf1597d53f793/pyserial-2.7.tar.gz#md5=794506184df83ef2290de0d18803dd11
- Insert SD card in the board, connect to USB debug console
- Once you boot and login (user root), enter enum34, erpc and pyserial directories
and run in each of them: python setup.py install
- now you can use: python run.py -p <PORT> -b <BAUDRATE>, to run the example

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini USB cable
- FRDM-K22F board
- Personal Computer

Board settings
==============
Connect J24-11(DAC_OUT) to J24-2(PTB0/ADC0_SE8)


Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Download the program to the target board.
3.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
4.  Run "run.py" on your PC and select COM port.

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================
Selected UART port: COM12, 115200 bd
FRDM-K22F
->Press '1' for DAC ADC conversion
->Press '2' for GPIO LED
->Press '3' for Accelerometer and Magnetometer
1
Please enter voltage <0V - 3.3V>: 1.5
You entered 1.5
Read value from ADC: 1.498535
---------------------
->Press '1' for DAC ADC conversion
->Press '2' for GPIO LED
->Press '3' for Accelerometer and Magnetometer
2
->Press '1' for red LED
->Press '2' for blue LED
->Press '3' for green LED
3
->Press '1' for DAC ADC conversion
->Press '2' for GPIO LED
->Press '3' for Accelerometer and Magnetometer
3
To calibrate Magnetometer and Accelerometer, press Enter

Roll the board on all orientations to get max and min values
Read value from Magnetometer:
    Accelerometer X axis: -92.000000
    Accelerometer Y axis: -64.000000
    Accelerometer Z axis: 8144.000000
    Magnetometer X axis: 199.000000
    Magnetometer Y axis: 143.000000
    Magnetometer Z axis: 526.000000
->Press '1' for DAC ADC conversion
->Press '2' for GPIO LED
->Press '3' for Accelerometer and Magnetometer
