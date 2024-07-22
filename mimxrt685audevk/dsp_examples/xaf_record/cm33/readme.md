Overview
========
The dsp_xaf_record application demonstrates audio processing using the DSP core,
the Xtensa Audio Framework (XAF) middleware library.

When the application is started, a shell interface is displayed on the terminal
that executes from the ARM application. User can control this with shell
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
set appropriate preprocessor define in project settings to 0/1 (e.g. XA_VIT_PRE_PROC etc.).
Debug and flash targets have full functionality enabled.

In order to get TRACE debug output from the XAF it is necessary to define XF_TRACE 1 in the project settings.
It is possible to save the TRACE output into RAM using DUMP_TRACE_TO_BUF 1 define on project level.
Please see the initalization of the TRACE function in the xaf_main_dsp.c file.
For more details see XAF documentation.

For custom VIT model generation (defining own wake words and voice commands) please use https://vit.nxp.com/


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
- MIMXRT685-AUD-EVK board
- Personal Computer
- Headphones with 3.5 mm stereo jack
- 8CH-DMIC expansion board - optional (rev B required)

Board settings
==============
1. Set the hardware jumpers (Tower system/base module) to default settings.
2. Set the hardware jumpers JP2 2-3, JP44 1-2 and JP45 1-2.

8CH-DMIC expansion board settings
1. Set the hardware jumpers on the 8-DMIC expansion board to 2MIC, 3MICA, 3MICC config
   (Short: J6, J9, J10).
2. Set the hardware jumpers JP44 2-3 and JP45 2-3 on the IMXRT685-AUD-EVK board for
   on-board DMIC bypass.

Prepare the Demo
================
1. Select how many microphones should be used
    - Set the BOARD_DMIC_NUM preprocessor macro to 1,2, 3 (default) or 4 in the project for the
      CM33 core.
    - When the 8CH-DMIC expansion board is used, the DMIC_BOARD_CONNECTED macro must be set to 1
      (default) in the project for the DSP core.
    Important:
        When you set the value to 2, 3 or 4 you have to connect the 8CH-DMIC expansion board to
        the MIMXRT685-AUD-EVK board to the DMIC connector (J31) and set the DMIC_BOARD_CONNECTED
        macro to 1.
        For safety reasons, the expansion board must be connected when the power supply is
        disconnected. Don't forget set the hardware jumpers JP44 2-3 and JP45 2-3.

2. Connect headphones to Audio HP / Line-Out connector (J4).
    - J50 connector (for third channel) can also be used when using 3 microphones.
3. Connect a micro USB cable between the PC host and the debug USB port (J5) on the board.
4. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5. Download the program for CM33 core to the target board.
6. Launch the debugger in your IDE to begin running the demo.
7. If building debug configuration, start the xt-ocd daemon and download the program for DSP core
   to the target board.
8. If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to begin running the
   demo.

### Notes
- DSP image can only be debugged using J-Link debugger. See again
'Getting Started with Xplorer for MIMXRT685-AUD-EVK.pdf' for more information.
- To be able to build the DSP project, please see the document
'Getting Started with Xplorer for MIMXRT685-AUD-EVK.pdf'.

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display the following:
```
    ******************************
    DSP audio framework demo start
    ******************************

    Configure CS42448 codec
    [APP_DSP_IPC_Task] start
    [APP_Shell_Task] start

    Copyright 2022 NXP
    >>
```

Demo commands:
```
"help": List all the registered commands

"record_dmic": Record DMIC audio, perform voice recognition (VIT) and playback on codec
  For voice recognition say supported WakeWord and in 3s frame spported command.
  List of supported commands:
  MUTE, NEXT, SKIP, PAIR_DEVICE, PAUSE, STOP, POWER_OFF, POWER_ON, PLAY_MUSIC
  PLAY_GAME, WATCH_CARTOON, WATCH_MOVIE
  NOTE: this command does not return to the shell
```

When record_dmic command runs successfully, the terminal will display following output:
```
    To see VIT functionality say wakeword and command.
```

Xtensa IDE log of successful start of command:
```
    Number of channels 1, sampling rate 16000, PCM width 16
    Audio Device Ready
    connected CAPTURER -> GAIN_0
    connected CAPTURER -> XA_VIT_PRE_PROC_0
    connected XA_VIT_PRE_PROC_0 -> XA_RENDERER_0
```

Running the demo DSP
====================
Debug configuration:
When the demo runs successfully, the terminal will display the following:
```
    Cadence Xtensa Audio Framework
      Library Name    : Audio Framework (Hostless)
      Library Version : 3.2
      API Version     : 3.0

    [DSP_Main] start
    [DSP_Main] established RPMsg link
    Number of channels 3, sampling rate 16000, PCM width 16

    Audio Device Ready
    VoiceSeekerLight lib initialized!
      version = 0.6.0
      num mics = 3
      max num mics = 4
      mic0 = (35, 0, 0)
      mic1 = (-35, 0, 0)
      mic2 = (0, -35, 0)
      num_spks = 0
      max num spks = 2
      samplerate = 16000
      framesize_in = 32
      framesize_out = 480
      create_aec = 0
      create_doa = 0
      buffer_length_sec = 1.5
      aec_filter_length_ms = 0
      VoiceSeekerLib allocated 80592 persistent bytes
      VoiceSeekerLib allocated 3840 scratch bytes
      Total                 = 72400 bytes

    connected CAPTURER -> GAIN_0
    connected XA_GAIN_0 -> XA_VOICE_SEEKER_0
    connected XA_VOICE_SEEKER_0 -> XA_VIT_PRE_PROC_0
    connected XA_VIT_PRE_PROC_0 -> XA_RENDERER_0
```

