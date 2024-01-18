Overview
========
This example shows how to use the PXP to process pixels, apply waveform mode
then use the EPDC to drive the E-INK panel.

In this example, first an image of RGB888 pixel format is processed by PXP to generate Y4 pixel data.
Then PXP waveform engine will apply the waveform mode 2 to the pixel data for EPDC to update high quality
image to the screen. The same image is updated to screen 4 times on different locations. Collision shall occur
and non-collided update(s) will appear on screen together within one fresh, the collided update(s) will
wait until the previous update(s) to finish then refresh.
Next the panel is cleared and PXP waveform engine will constantly apply the waveform mode 1 to black dot for
EPDC to draw line on panel to simulate pen input. The update shall be non-flashy and quicker than image update.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- MIPI-EPDC panel VB3300-FOC

Board settings
==============
Connect the MIPI panel to MIMX8ULP-EVK EPDC socket according to the instruction printed to console.

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Build the project
5.  Generate flash.bin with imx-mkimage and download it according to Getting Started doc
6.  Open two serials lines provided at J17 USB port.
    (e.g. /dev/ttyUSB0~3, /dev/ttyUSB2 for A Core, /dev/ttyUSB3 for M Core)
7.  Press the reset button on your board and boot to uboot and let uboot not using diplay peripherals
    => setenv video_off yes;saveenv
    then re-power on the board
8.  Let Stop in Uboot.

Running the demo
================
When the example runs, the screen shows what described in overview.
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EPDC example start...

Connect the MIPI panel to MIMX8ULP-EVK EPDC socket.

Press any key to power on panel.

Initializing the panel...

Displaying the image to screen 4 times.

Non-collided update(s) will appear on screen together within one fresh, the collided update(s) will wait until the previous update(s) to finish then refresh.

Drawing lines to panel...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
