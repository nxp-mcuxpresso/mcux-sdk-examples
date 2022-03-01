LPC5460x USB Audio Example
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


Special connection requirements
-------------------------------
For downloading code and debugging connect an USB cable to micro
connector (J8) and host.
Connect another USB cable between micro connector (J3) on board
and to a host.
Connect headphone/speakers to audio output on shield board.


Build procedures:
----------------
Visit the 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.

Note:
When recorded throught line in cable and there is some sound distortion which may be related 
with 24bit ADC converter overload with codec's input signal, codec's default background noise
(or the current noise from the board) even if no sound input or the quality of the line in cable 
and this is inevitable.
