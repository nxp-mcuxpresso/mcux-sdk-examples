Aruba_Flashless_MCM USB Virtual Comm port Example
======================================

Example description
-------------------
The example shows how to use USBD ROM stack to create a virtual comm port.
When the USB on J3 is connected to a PC, the host would recognize the USB connection as 
a new serial port. Using a serial terminal communication between the host and LPCXpresso54S018 can be established.


Special connection requirements
-------------------------------
For downloading code and debugging connect an USB cable to micro
connector (J8) and host.
Connect another USB cable between micro connector (J3) on board
and to a host.


Build procedures:
----------------
Visit the 'LPCOpen quickstart guides' [http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]
to get started building LPCOpen projects.

Installing the CDC driver for usb_rom_device_cdc example:
please refer to the readme of usb_device_cdc_vcom example.
NOTE: please use the inf in usb_rom_device_cdc\bm\inf when select the inf file.
Change log:
-----------
1. Fix send throguhput does not match with receive issue.
   this issue is fixed from these two aspects, we removed one memcpy operation in the sending process 
   and we moved the data receiving process from the ISR to the task, then we found the sending throughput 
   is almost same as receiving throughput, then this issue disappeared.


