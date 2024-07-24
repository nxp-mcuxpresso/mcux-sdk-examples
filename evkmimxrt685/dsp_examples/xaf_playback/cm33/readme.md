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
- cm33/ is the ARM application for the Cortex-M33 core
- dsp/ is the DSP application for the DSP core

The demo will combine both applications into one ARM image.
With this, the ARM core will load and start the DSP application on
startup. Pre-compiled DSP binary images are provided under dsp/binary/ directory.
If you make changes to the DSP application, rebuild the ARM application after building the DSP application.
If you plan to use MCUXpresso IDE for Cortex-M33 you will have to make sure that
the preprocessor symbol DSP_IMAGE_COPY_TO_RAM, found in IDE project settings,
is defined to the value 1 when building release configuration.

The DSP application can be built by the following tools:
Xtensa Xplorer or Xtensa C Compiler. Required tool versions can be found
in MCUXpresso SDK Release Notes for the board. Application for Cortex-M33 can be built by the other toolchains listed there.
The ARM application will power and clock the DSP, so it must be loaded prior to loading the DSP application.

In order to debug both the Cortex-M33 and DSP side of the application, please follow the instructions:
1. It is necessary to run the Cortex-M33 side first and stop the application before the DSP_Start function
2. Run the xt-ocd daemon with proper settings
3. Download and debug the DSP application

In order to get TRACE debug output from the XAF it is necessary to define XF_TRACE 1 in the project settings.
It is possible to save the TRACE output into RAM using DUMP_TRACE_TO_BUF 1 define on project level.
Please see the initalization of the TRACE function in the xaf_main_dsp.c file.
For more details see XAF documentation.

Known issues
The "file stop" command doesn't stop the playback for some small files (with low sample rate).


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
- JTAG/SWD debugger
- EVK-MIMXRT685 board
- Personal Computer
- Headphones with 3.5 mm stereo jack

Board settings
==============
To enable the example audio using WM8904 codec, connect pins as follows:
- JP7-1        <-->        JP8-2

Prepare the Demo
================
1. Connect headphones to Audio HP / Line-Out connector (J4).
2. Connect a micro USB cable between the PC host and the debug USB port (J5) on the board
3. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
4. Download the program for CM33 core to the target board.
5. Launch the debugger in your IDE to begin running the demo.
6. If building debug configuration, start the xt-ocd daemon and download the program for
   DSP core to the target board.
7. If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to
begin running the demo.

### Notes
- DSP image can only be debugged using J-Link debugger. See again
'Getting Started with Xplorer for EVK-MIMXRT685.pdf' for more information.
- To be able to build the DSP project, please see the document
'Getting Started with Xplorer for EVK-MIMXRT685.pdf'.

Running the deeemo CM33
When the demo runs successfully, the terminal will display the following:
```
    ******************************
    DSP audio framework demo start
    ******************************

    [CM33 Main] Configure WM8904 codec
    [CM33_Main] DSP image copied to DSP TCM
    [CM33_Main][APP_SDCARD_Task] start
    [CM33_Main][APP_DSP_IPC_Task] start
    [CM33_Main][APP_Shell_Task] start

    Copyright  2022  NXP
    >>
```

Demo commands:
```
"help": List all the registered commands

"file": Perform audio file decode and playback on DSP
  USAGE: file [list|stop|<audio_file>]
    list          List audio files on SD card available for playback
    <audio_file>  Select file from SD card and start playback
```

When file command starts playback successfully, the terminal will display following output:
```
    [APP_DSP_IPC_Task] response from DSP, cmd: 12, error: 0
    DSP file playback start
    >>
```

Xtensa IDE log when command is playing a file (mp3/aac/vorbis):
```
    [DSP_Main] File playback start, initial buffer size: 16384

    [DSP Codec] Audio Device Ready

    [DSP Codec] Decoder component started

    [DSP Codec] Setting decode playback format:

    [DSP Codec] Decoder    : mp3_dec

    [DSP Codec] Sample rate: 48000

    [DSP Codec] Bit Width  : 16

    [DSP Codec] Channels   : 2

    [DSP Codec] Renderer component started

    [DSP Codec] Connected XA_DECODER -> XA_RENDERER

    [DSP_ProcessThread] start

    [DSP_BufferThread] start


    [DSP_Main] File playback stop

    [DSP_ProcessThread] exiting

    [DSP_BufferThread] exiting

    [DSP Codec] Audio device closed

  Xtensa IDE will not show any additional log entry.
```

Running the demo DSP
====================
Debug configuration:
When the demo runs successfully, the terminal will display the following:
```
    [DSP_Main] Cadence Xtensa Audio Framework

    [DSP_Main] Library Name    : Audio Framework (Hostless)

    [DSP_Main] Library Version : 3.2

    [DSP_Main] API Version     : 3.0



    [DSP_Main] start

    [DSP_Main] established RPMsg link
```

