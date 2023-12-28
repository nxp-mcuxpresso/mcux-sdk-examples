Overview
========
In this demo, A core decoded music data and put it to DDR buffer and informs M core with the related information. 
Then M core will take the ownership of consuming the buffer, it will copy buffer from DDR to TCM, manipulating SDMA to transfer the data to SAI and codec for playback. 
It gives DDR and A core opportunity to do power saving for rather long time frame. M core will also take ownership of codec initialization.
SRTM(Simplified Real Time Messaging) protocol is used to communicate between A core and M core. 
The protocol provides various commands for A core and M core to communicate with each other. 
If there is no audio palyback, M core will enter the STOP mode, and the whole SOC system would enter deep sleep mode(DSM) once A core enter low power status.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer
- Headphone

Board settings
==============
No special settings are required.

#### Note! ####
1.  This case does not support ddr and flash target. 
2.  This case runs together with Linux and the Linux release version should not be lower than 5.15.32-2.0.0.

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
4.  Download the program to the target board.
5.  Turn on the power switch on your board to begin running the demo.

******************
NOTE
******************
1.  The 16/24/32bits for PCM Music stream are supported.
2.  Since the music files are typically large, users could create a new large size partition in the SD card to place the music files.
3.  After M core running, please boot the linux kernel to create the rpmsg channel between A core and M core.
    Make sure the FDT file and key bootargs are correctly set before booting the linux kernel. Following is the example commands on A core U-Boot console:
      =>setenv fdtfile imx8ulp-evk-lpa.dtb
      =>setenv mmcargs 'setenv bootargs ${jh_clk} console=${console} root=${mmcroot} no_console_suspend snd_pcm.max_alloc_per_card=134217728'
      =>saveenv
4.  Please make sure audio file (such as xxx.wav) in any filesystem that Linux can access.
    One of the way to send audio file to Linux filesystem:
    Under A core U-Boot console, execute below command to enable 8ULP board's USB device mode (USB Mass Storage) function:
      u-boot->ums 0 mmc 0
    Connect TYPE-C USB cable between 8ULP board and PC. You will see a drive show on PC to allow copy any file to board.
    After A core boot to Linux console, the audio file is located at /run/media/boot-mmcblk0p1/

******************
Playback command
******************
Note:
1. Please use the command "cat /proc/asound/cards" to check the wm8960 sound card number (on A core Linux console).
E.g: Type command:
        ~# cat /proc/asound/cards
     The available sound cards can be shown:

     0 [btscoaudio     ]: simple-card - bt-sco-audio
                          bt-sco-audio
     1 [imxspdif       ]: imx-spdif - imx-spdif
                          imx-spdif
     2 [wm8960audio    ]: wm8960-audio - wm8960-audio
                          wm8960-audio

Then the wm8960 sound card number is 3.

When playback the .wav file (on A Core Linux console):
1.  If you want to playback with pause/resume command, below command can be used:
      "aplay -Dhw:2 -i xxx.wav -N";
    press space key on the keyboard to pause, and press the space key again to resume.
2.  8ULP supports audio format of S16_LE, and sample rates could be 8kHz, 16kHz and 48kHz, NOT including 44.1kHz.
3.  Current demo should NOT be run with argument 'plug', otherwise LPA mode will NOT take effect:
      "aplay -Dplughw:2 -i xxx.wav -N"
4.  If you want to playback with APD low power mode and specified period-size, below command can be used:
      "aplay -Dhw:2 --buffer-size=xxx --period-size=xxx xxx.wav -N &" or
      "aplay -Dhw:2 --buffer-time=xxx --period-time=xxx xxx.wav -N &".
    E.g: "aplay -Dhw:2 --period-time=500000 --buffer-time=10000000 xxx.wav -N &"
    Now please use "echo mem > /sys/power/state" command to make APD enter suspend mode and the playback work normally.
    Note:
	 1. For APD into power down mode, this demo support dual boot mode only.
	 2. Please enable the A core has enough time to fill the audio buffer before going into suspend mode.

Running the demo
================
When the demo runs successfully, the M core log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
####################  LOW POWER AUDIO TASK ####################

    Build Time: Aug 30 2022--12:56:26
Start SRTM communication
********************************
 Wait the Linux kernel boot up to create the link between M core and A core.

********************************
BOARD_ReleaseTRDC: 75 start release trdc
BOARD_ReleaseTRDC: 78 finished release trdc, status = 0xd6
BOARD_SetTrdcGlobalConfig: 93 start setup trdc
BOARD_SetTrdcGlobalConfig: 411 finished setup trdc
Handle Peer Core Linkup
The rpmsg channel between M core and A core created!
********************************


Task A is working now.



