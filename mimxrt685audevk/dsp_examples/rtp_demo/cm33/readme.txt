Overview
========
The dsp_rtp_demo application demonstrates playback of G.711 encoded RTP streams received via WiFi.

The application can play the following types of RTP streams:
- G.711 μ-law encoding, 1 channel, 8 bits per sample, 8 kHz sample rate (RTP payload type 0 - PCMU)
- G.711 A-law encoding, 1 channel, 8 bits per sample, 8 kHz sample rate (RTP payload type 8 - PCMA)
Data from streams in other formats, including stereo streams, are discarded.

This demo contains two applications:
- cm33/ is the ARM application for Cortex-M33 core
- dsp/ is the DSP application for HiFi4 core

ARM application is responsible for connecting to the WiFi network, configuring board HW audio codec and receiving UDP packets with RTP payload on a multicast address or unicast address obtained from WiFi network.
The RTP payload is received into the shared memory buffers and other core is notified about it using RPMsg-Lite IPC.

DSP application running on HiFi4 core uses Xtensa Audio Framework (XAF) middleware library for audio processing which is realized using a chain of connected components. The individual frames from received RTP streams are referenced in the sorted lists which work as buffers. The frames are taken from the lists, decoded and inserted into the audio processing chain when needed. Up to 4 concurrent streams are supported. Audio is played through headphone output. Application handles packets received out of order, ignores duplicate packets and uses Packet Loss Concealment (PLC) to fill-in the dropped frames with synthesized audio signal similar to the preceding output. PLC maintains smoother output than what would inserting a silence or looping the last frame would lead into.

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

To be able to build the DSP project, please see the document
'Getting Started with Xplorer for EVK-MIMXRT685.pdf'.

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM


Customization
It is necessary to configure WiFi network to which the ARM application will connect.
The following symbol definitions could be overriden:

WIFI_SSID - WiFi network to join.
WIFI_PASSWORD - Password needed to join WiFi network.
RTP_RECV_MCAST_ADDRESS - RTP stream multicast address as IPv4 address string. Set to NULL if you want to receive unicast only.
RTP_RECV_PORT - UDP port number to listen to incoming RTP stream on.
RTP_DEBUG - Enable debugging messages for RTP. Set to LWIP_DBG_ON or LWIP_DBG_OFF to enable or disable respectively.

The following symbol definitions could be overriden for DSP application:

PLC_ENABLED - Enable Packet Loss Concealment (PLC) in G.711 codec library. Set to 1 for enabled, 0 to disable.
MAX_INPUT_STREAMS - Maximum number of simultaneously played streams (1 - 4).
BUFFERING_FRAMES - Minimum number of RTP frames (each is 10 ms long) to receive from each stream before its playback can start.
Higher value may be needed for smoother playback if there is higher jitter in UDP packet reception time. Smaller value means shorter delay in playback.
If the buffer is emptied during playback, it will wait until BUFFERING_FRAMES number of frames is received again before playback will continue.
The consequence is that streams with less frames than BUFFERING_FRAMES value will not be played at all.
FRAME_LIST_SIZE - Maximum number of frames (10 ms long) buffered for a session. The maximum value is limited by the number of frames which can fit into shared IPC memory
and also by heap memory available for frame descriptors. Needs to be set to at least BUFFERING_FRAMES value.

Streaming setup
In order to stream audio, follow the steps below:

1. Open VLC media player on a supported device/PC connected to the same WiFi network as the target.
2. To broadcast a stream over the network, click on 'Media' -> 'Stream'.
3. In the Open Media dialog, File tab, click on 'Add' button to select the media to stream.
   It can be WAV file, the format must be G.711 μ-law or A-law encoding, 1 channel, 8 bits per sample, 8 kHz sample rate, otherwise the board will not play it.
   Click on 'Stream' after selection.
4. In the Stream Output dialog, select 'Next'.
5. In the Destination Setup window, select 'New destination' -> 'RTP Audio/Video Profile', then select 'Add'.
6. Set the address to to the value of RTP_RECV_MCAST_ADDRESS (which is 232.0.232.232 by default) and the base port to the value of RTP_RECV_PORT (which is 5004 by default).
   It is also possible to use the unicast IP address that the target acquired from DHCP. Click 'Next'.
7. In the Transcoding Options window, uncheck 'Activate Transcoding'. Select 'Next'.
8. To start streaming, click the 'Stream' button once the demo is running on the target board.
9. Listen to the streamed audio being played via target board headphone output.

Multiple instances of VLC media player could be opened to test mixed playback of concurrent streams.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- 2 x Micro USB cable
- JTAG/SWD debugger (optional)
- MIMXRT685-AUD-EVK board
- Personal Computer
- Headphones with 3.5 mm stereo jack
- WiFi access point with DHCP server


Board settings
==============

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect the WiFi module to SD card slot.
3.  Connect a micro USB cable between the PC host and the debug USB port (J5) on the board
4.  Connect a micro USB cable between the USB power source (PC host port, active USB hub or other source) and the external power USB port (J6) on the board (recommended)
5.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
6.  Ensure the device running VLC and the target board are connecting to the same WiFi network
7.  Download the program for CM33 core to the target board.
8.  Launch the debugger in your IDE to begin running the demo.
9.  If building debug configuration, start the xt-ocd daemon and download the program for DSP core to the target board.
10.  If building debug configuration, launch the Xtensa IDE or xt-gdb debugger to begin running the demo.

NOTE: DSP image can only be debugged using J-Link debugger.  See
'Getting Started with Xplorer for MIMXRT685-AUD-EVK.pdf' for more information.
Running the demo
================

Running the demo CM33
=====================
When the demo runs successfully, the terminal will display similar output:

    ******************************
    RTP demo start
    ******************************

    Configure [board specific] codec
    [app_dsp_ipc] start
    [wifi_client_task] start
    Initializing WLAN
    [rtp_receiver_task] start
    MAC Address: 00:13:43:7F:9C:D7
    Starting WLAN
    [net] Initialized TCP/IP networking stack
    WLAN initialized
    WLAN FW version: IW416-V0, RF878X, FP91, 16.91.10.p214, WPA2_CVE_FIX 1, PVE_FIX 1
    Connecting to "test-network"
    Connected to "test-network" with IP = [192.168.0.3]
    [wifi_client_task] done

Running the demo DSP
====================
Debug configuration:
When the demo runs successfully, the terminal will display the following:

    [main_dsp] start

    Cadence Xtensa Audio Framework
      Library Name    : Audio Framework (Hostless)
      Library Version : 2.6p2
      API Version     : 2.0

    G.711
      Library Version : 1.3p1
      API Version     : 1.0

    [main_dsp] established RPMsg link
    [dsp_xaf] start
    [dsp_xaf] Audio device open
    [dsp_xaf] STEREO component created
    [dsp_xaf] STEREO component created
    [dsp_xaf] STEREO component created
    [dsp_xaf] STEREO component created
    [dsp_xaf] MIXER component created
    [dsp_xaf] Connected STEREO -> MIXER
    [dsp_xaf] Connected STEREO -> MIXER
    [dsp_xaf] Connected STEREO -> MIXER
    [dsp_xaf] Connected STEREO -> MIXER
    [dsp_xaf] MIXER component started
    [dsp_xaf] SRC component created
    [dsp_xaf] Connected MIXER -> SRC
    [dsp_xaf] SRC component started
    [dsp_xaf] RENDERER component created
    [dsp_xaf] Connected SRC -> RENDERER

Running the demo - streaming
============================
Once both CM33 and DSP applications are running as described above, follow the steps for streaming setup
and listen to the streamed audio being played via target board headphone output.
