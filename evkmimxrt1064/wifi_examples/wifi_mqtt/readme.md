Overview
========
The wifi_mqtt demo application demonstrates an MQTT client on the lwIP TCP/IP stack with FreeRTOS.
The board connects to the broker and sends few messages.

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM



SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT1064 board
- Personal Computer


Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Connect the Wi-Fi module.
4.  Prior building of example you may want to set following symbols:
	- WIFI_SSID and WIFI_PASSWORD according settings of your AP.
	- EXAMPLE_MQTT_SERVER_HOST and EXAMPLE_MQTT_SERVER_PORT to aim to your broker.
5.  Download the program to the target board.
6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs, the log would be seen on the terminal like:

************************************************
 MQTT client example
************************************************
[i] Initializing Wi-Fi connection... 
MAC Address: 48:E7:DA:9A:CE:39 
-------- wlan_event_callback 11 --------
[i] Successfully initialized Wi-Fi module
Connecting as client to ssid: my_network with password my_password
-------- wlan_event_callback 0 --------
[i] Connected to Wi-Fi
ssid: my_network
[!]passphrase: my_password

IPv4 Address     : 192.168.0.249
IPv4 Subnet mask : 255.255.255.0
IPv4 Gateway     : 0.0.0.0

Resolving "broker.hivemq.com"...
Connecting to MQTT broker at 18.185.216.165...
MQTT client "nxp_f50003c25757bd58aa6d0ce50102f020" connected.
Subscribing to the topic "lwip_topic/#" with QoS 0...
Subscribing to the topic "lwip_other/#" with QoS 1...
Subscribed to the topic "lwip_topic/#".
Subscribed to the topic "lwip_other/#".
Going to publish to the topic "lwip_topic/100"...
Published to the topic "lwip_topic/100".
Received 18 bytes from the topic "lwip_topic/100": "message from board"
Going to publish to the topic "lwip_topic/100"...
Published to the topic "lwip_topic/100".
Received 18 bytes from the topic "lwip_topic/100": "message from board"
Going to publish to the topic "lwip_topic/100"...
Published to the topic "lwip_topic/100".
Received 18 bytes from the topic "lwip_topic/100": "message from board"
Going to publish to the topic "lwip_topic/100"...
Published to the topic "lwip_topic/100".
Received 18 bytes from the topic "lwip_topic/100": "message from board"
Going to publish to the topic "lwip_topic/100"...
Published to the topic "lwip_topic/100".
Received 18 bytes from the topic "lwip_topic/100": "message from board"
