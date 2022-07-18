Overview
========
The dsp_xaf_playback application demonstrates audio processing using the DSP core,
the Xtensa Audio Framework (XAF) middleware library, and select Xtensa audio
codecs.

When the application is started, a shell interface is displayed on the terminal
that executes from the ARM application.  User can control this with shell
commands which are relayed via RPMsg-Lite IPC to the DSP where they are
processed and response is returned.

Type "help" to see the command list.

This demo contains two applications:
- cm33/ is the ARM application for Cortex-M33 core
- dsp/ is the DSP application for DSP core

The release configurations of the demo will combine both applications into one ARM
image. With this, the ARM core will load and start the DSP application on
startup. Pre-compiled DSP binary images are provided under dsp/binary/ directory.
If you make changes to the DSP application in release configuration, rebuild
ARM application after building the DSP application.
If you plan to use MCUXpresso IDE for cm33 you will have to make sure that
the preprocessor symbol DSP_IMAGE_COPY_TO_RAM, found in IDE project settings,
is defined to the value 1 when building release configuration.

The debug configurations will build two separate applications that need to be
loaded independently. DSP application can be built by the following tools:
Xtensa Xplorer or Xtensa C Compiler. Required tool versions can be found
in MCUXpresso SDK Release Notes for the board. Application for cm33 can be built
by the other toolchains listed there. If you plan to use MCUXpresso IDE for cm33
you will have to make sure that the preprocessor symbol DSP_IMAGE_COPY_TO_RAM,
found in IDE project settings, is defined to the value 0 when building debug configuration.
The ARM application will power and clock the DSP, so it must be loaded prior to
loading the DSP application.

There are limited features in release SRAM target because of memory limitations. To enable/disable components,
set appropriate preprocessor define in project settings to 0/1 (e.g. XA_VORBIS_DECODER, XA_OPUS_ENCODER, etc.).
Debug and flash targets have full functionality enabled.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- EVK-MIMXRT595 board
- Personal Computer
- Headphones with 3.5 mm stereo jack

Board settings
==============
Note: The I3C Pin configuration in pin_mux.c is verified for default 1.8V, for 3.3V,
need to manually configure slew rate to slow mode for I3C-SCL/SDA.

To enable the example audio using WM8904 codec, connect pins as follows:
  JP7-1        <-->        JP8-2

Prepare the Demo
================
NOTE: To be able to build the DSP project, please see the document
'Getting Started with Xplorer for EVK-MIMXRT595.pdf'.

1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect a micro USB cable between the PC host and the debug USB port (J40) on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program for CM33 core to the target board.
5.  Launch the debugger in your IDE to begin running the demo.
6.  If building debug configuration, start the xt-ocd daemon and download the program for DSP core to the target board.
7.  If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to
begin running the demo.

NOTE: DSP image can only be debugged using J-Link debugger.  See again
'Getting Started with Xplorer for EVK-MIMXRT595.pdf' for more information.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:

    ******************************
    DSP audio framework demo start
    ******************************

    Configure WM8904 codec
    [APP_SDCARD_Task] start
    [APP_DSP_IPC_Task] start
    [APP_Shell_Task] start

    SHELL build: Dec 18 2019
    Copyright  2021  NXP
    >>

Demo commands:

"help": List all the registered commands

"file": Perform audio file decode and playback on DSP
  USAGE: file [list|stop|<audio_file>]
    list          List audio files on SD card available for playback
    <audio_file>  Select file from SD card and start playback

  When file command starts playback successfully, the terminal will display following output:
    [APP_DSP_IPC_Task] response from DSP, cmd: 12, error: 0
    DSP file playback start
    >>

  Xtensa IDE log when command is playing a file:
    File playback start, initial buffer size: 16384
    [DSP Codec] Audio Device Ready
    [DSP Codec] Decoder component started
    [DSP Codec] Setting decode playback format:
      Decoder    : mp3_dec
      Sample rate: 16000
      Bit Width  : 16
      Channels   : 2
    [DSP Codec] EAP filter component started
    [DSP Codec] Renderer component started
    [DSP Codec] Connected XA_DECODER -> XA_EAP_FILTER
    [DSP Codec] Connected XA_EAP_FILTER -> XA_RENDERER
    [DSP_ProcessThread] start
    [DSP_BufferThread] start

"eap": Set EAP parameters
  USAGE: eap [1|2|3|4|5|6|7|+|-|l|r]
  OPTIONS:
    1:  All effect Off
    2:  Voice enhancer
    3:  Music enhancer
    4:  Auto volume leveler
    5:  Loudness maximiser
    6:  3D Concert sound
    7:  Custom
    +:  Volume up
    -:  Volume down
    l:  Balance left
    r:  Balance right

  When eap command runs successfully, the terminal will display the following:
    [APP_DSP_IPC_Task] response from DSP, cmd: 17, error: 0
    DSP Filter cfg success!

  Xtensa IDE will not show any additional log entry.

Running the demo DSP
====================
Debug configuration:
When the demo runs successfully, the terminal will display the following:

    Cadence Xtensa Audio Framework
      Library Name    : Audio Framework (Hostless)
      Library Version : 2.6p1
      API Version     : 2.0

    [DSP_Main] start
    [DSP_Main] established RPMsg link

