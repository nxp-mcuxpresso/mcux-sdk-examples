Overview
========
Xtensa Audio Framework example with TensorFlow Lite Micro word detection
recognizing 4 categories: "silence", "unknown", "yes", "no".


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT685 board
- Personal computer

Board settings
==============

Prepare the Demo
================
1. Build the HiFi4 project first to create the binary image.
2. Replace the DSP binary files in the Cortex-M33 project with the files
   generated into the <example_root>/hifi4/binary directory if necessary.
3. Continue with the Cortex-M33 master project.
4. Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
5. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
6. Download the program to the target board.
7. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the demo in the terminal window:

Starting Xtensa example from Cortex-M33 core
[main_dsp] Start

Cadence Xtensa Audio Framework
  Library Name    : Audio Framework (Hostless)
  Library Version : 2.6p2
  API Version     : 2.0

[main_dsp] Established RPMsg link
[dsp_xaf] Start
[dsp_xaf] Audio device open
[dsp_xaf] Capturer component created
[dsp_xaf] Capturer component started
[dsp_xaf] Renderer component created
[dsp_xaf] Connected capturer -> renderer
[dsp_xaf] Closing
[dsp_xaf] Audio device closed

[dsp_xaf] Audio device open
[dsp_xaf] Capturer component created
[dsp_xaf] Capturer component started
[dsp_xaf] Connected capturer -> microspeech FE
[dsp_xaf] Microspeech FE component started
[dsp_xaf] Connected microspeech FE -> inference
[dsp_xaf] Inference component started

Detected: unknown (78%)
Detected: unknown (80%)
Detected: unknown (91%)
Detected: yes (78%)
Detected: yes (80%)
Detected: no (79%)
Detected: unknown (78%)

