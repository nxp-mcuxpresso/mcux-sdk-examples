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
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMX8M Plus board
- J-Link Debug Probe
- 12V power supply
- Personal Computer
- Headphone
- 8MIC-PRI-MX8

Board settings
==============
Connect the 8MIC-PRI-MX8 Microphone board to J21 on the base board if sound capture is needed.

#### Note! ####
1.  This case does not support ddr and flash target. 
2.  This case runs together with Linux and the Linux release version should be not lower than 5.10.72-2.2.0.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW3 to power on the board
2.  Connect a USB cable between the host PC and the J23 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

******************
NOTE
******************
1.  The 16/24/32bits for PCM Music stream are supported.
2.  Since the music files are typically large, users could create a new large size patition in the SD card to place the music files.
3.  After M core running, please boot the linux kernel to create the rpmsg channel between A core and M core.
    Make sure the FDT file is correctly set before booting the linux kernel. The following command can be used to set FDT file in uboot console:
    u-boot=>setenv fdtfile imx8mp-evk-rpmsg.dtb
    u-boot=>saveenv
    For RevB4 board, use different dtb
    u-boot=>setenv fdtfile imx8mp-evk-revb4-rpmsg.dtb
    u-boot=>saveenv
    Set the "snd_pcm.max_alloc_per_card" in bootargs, use the following command to print default mmcargs and add "snd_pcm.max_alloc_per_card=134217728" to the end. 
    u-boot=>printenv mmcargs
        For example, "mmcargs=setenv bootargs ${jh_clk} ${mcore_clk} console=${console} root=${mmcroot}" is printed, then set the mmcargs using the following command. 
    u-boot=>setenv mmcargs 'setenv bootargs ${jh_clk} ${mcore_clk} console=${console} root=${mmcroot} snd_pcm.max_alloc_per_card=134217728'
    u-boot=>saveenv
    u-boot=>run prepare_mcore
4.  Please make sure there exists xxx.wav file in the SD card.
    If the music file is placed at the Windows FAT32 paritions, after the linux kernel boots up and logs on as root,
    using the "mount /dev/mmcblk1p1 /mnt" and then go to "/mnt" folder to playabck the music using the playback command.
    If the music file is placed at the Linux paritions, eg "/home", could playback the music directly using the playback command. 

******************
Playback/record command
******************
Note:
1. Please use the command "cat /proc/asound/cards" to check the wm8960 sound card number.
E.g: Type command:
        ~# cat /proc/asound/cards
     The available sound cards can be shown:

     0 [imxaudiomicfil ]: imx-audio-micfi - imx-audio-micfil
                      imx-audio-micfil
     1 [imxaudioxcvr   ]: imx-audio-xcvr - imx-audio-xcvr
                      imx-audio-xcvr
     2 [btscoaudio     ]: bt-sco-audio - bt-sco-audio
                      bt-sco-audio
     3 [wm8960audio    ]: wm8960-audio - wm8960-audio
                      wm8960-audio
     4 [audiohdmi      ]: audio-hdmi - audio-hdmi
                      audio-hdmi

Then the wm8960 sound card number is 3, the MICFIL sound card number is 0.

When playback the .wav file:
1.  If want to playabck with pause/resume command, could use command: 
      "aplay -Dhw:3 -i xxx.wav -N";
    press space key on the keyboard to pause, and press the space key again to resume.
2.  If want to playback with low power mode and specified period-size, could use command:
      "aplay -Dhw:3 --buffer-size=xxx --period-size=xxx xxx.wav -N &" or
      "aplay -Dhw:3 --buffer-time=xxx --period-time=xxx xxx.wav -N &".
    E.g: "aplay -Dhw:3 --period-time=500000 --buffer-time=10000000 xxx.wav -N &"
    Now please use "echo mem > /sys/power/state" command to make A core enter suspend mode and the playabck work normally.
    Note, make sure the A core has enough time to fill the audio buffer before going into suspend mode.

When recording sound, could use command:
	"arecord -Dhw:0,0 -r44100 -fS32_LE -c2 test.wav &"
	fS32_LE and 1-8 channels are supported.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
####################  LOW POWER AUDIO TASK ####################

    Build Time: Aug 20 2020--08:56:27  
********************************
 Wait the Linux kernel boot up to create the link between M core and A core.

********************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
M core is running now, please boot the linux kernel and use the command to playback music.




