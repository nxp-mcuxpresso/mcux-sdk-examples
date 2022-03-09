LPC5411x USB Audio Example
==========================

Example description
-------------------
The example shows how to use USBD ROM stack to create and USB AUDIO class
device. This device supports 2 channel audio in (MIC/LINE_IN) and 2 channel
audio out (SPEAKERS/HEADPHONE). Default supported format is 48000 Hz sampling
rate with 16-bit sample width. By default the two analog microphones on the
codec board will be used as input devices [stereo]. When the device is connected
it will be enumerated by the host as an audio device and user can play audio from
the host using any media player, the audio should be played on the headphone
connected. For recording audio open the voice recording application (example
in windows 10 user Voice Recorder App), to record audio from analog line input.


Supported boards
----------------
LPC51U68 LPCXpresso base board with Mic Audio OLED Shield Board.


Special connection requirements
-------------------------------
For USB to connect the jumper JP10 should be installed on the LPC5411x board.
For downloading code and debugging connect an USB cable to micro
connector (J7) and host.
Connect another USB cable between micro connector (J5) on board
and to a host.
Connect headphone/speakers to audio output on shield board.


Build procedures:
----------------
Visit the 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.
