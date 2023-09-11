Overview
========
This is the Wi-Fi WPA Supplicant example to demonstrate the CLI support usage using wpa supplicant. The CLI module allows user to add CLIs in application.
Currently only WLAN connection Manager CLIs are available.

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).

If Wi-Fi module Redfinch is used, please note that the application is for Redfinch A1 by default. If you are using Redfinch A0, please
undefine flag CONFIG_RW610_A1 in wifi_config.h before compiling application.

For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT685-AUD-EVK board
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
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
1. Add CLI init API in applications main function.
2. Add WLAN CLI init API once WLAN Connection Manager gets initialized correctly.
3. When the demo starts, a welcome message would appear on the terminal, press enter for command prompt:
   Press tab or type help to list out all available CLI commands.

   wifi wpa supplicant demo
   Initialize CLI
   Initialize WLAN Driver
   MAC Address: 80:D2:1D:E8:2F:2F
   Info: supplicant_main_task: 241 Starting wpa_supplicant thread with debug level: 3
   Info: Successfully initialized wpa_supplicant
   Info: iface_cb: iface ml1 ifindex 2 80:d2:1d:e8:2f:2f
   Info: Using interface ml1
   Info: Initializing interface 0: ml1
   app_cb: WLAN: received event 11
   app_cb: WLAN initialized
   WLAN CLIs are initialized
   CLIs Available:
   help
   wlan-reset
   wlan-version
   wlan-mac
   wlan-thread-info
   wlan-net-stats
   wlan-set-mac <MAC_Address>
   wlan-scan
   wlan-scan-opt ssid <ssid> bssid ...
   wlan-add <profile_name> ssid <ssid> bssid...
   wlan-remove <profile_name>
   wlan-list
   wlan-connect <profile_name>
   wlan-start-network <profile_name>
   wlan-stop-network
   wlan-disconnect
   wlan-stat
   wlan-info
   wlan-address
   wlan-get-uap-channel
   wlan-get-uap-sta-list
   wlan-ieee-ps <0/1>
   wlan-deep-sleep-ps <0/1>
   wlan-wnm-ps <0/1> <sleep_interval>
   wlan-set-max-clients-count <max clients count>
   wlan-rts <sta/uap> <rts threshold>
   wlan-frag <sta/uap> <fragment threshold>
   wlan-host-11k-enable <0/1>
   wlan-host-11k-neighbor-req [ssid <ssid>]
   wlan-host-11v-bss-trans-query <0..16>
   wlan-pmksa-list
   wlan-pmksa-flush
   wlan-set-scan-interval <scan_int: in seconds>
   wlan-sta-filter  <filter mode> [<mac address list>]
   wlan-roaming <0/1>
   wlan-multi-mef <ping/arp/multicast/del> [<action>]
   wlan-host-sleep <0/1> mef/[wowlan <wake_up_conds>]
   wlan-send-hostcmd
   wlan-set-uap-bandwidth <1/2/3> 1:20 MHz 2:40MHz 3:80MHz
   wlan-set-uap-hidden-ssid <0/1/2>
   wlan-ft-roam <bssid> <channel>
   wlan-set-antcfg <ant mode> [evaluate_time]
   wlan-get-antcfg
   wlan-scan-channel-gap <channel_gap_value>
   wlan-set-regioncode <region-code>
   wlan-get-regioncode
   wlan-rssi-low-threshold <threshold_value>
   wlan-generate-wps-pin
   wlan-start-wps-pbc
   wlan-start-wps-pin <8 digit pin>
   wlan-wps-cancel
   wlan-start-ap-wps-pbc
   wlan-start-ap-wps-pin <8 digit pin>
   wlan-wps-ap-cancel
   wlan-get-signal
   wlan-set-forceRTS <0/1>
   wlan-cloud-keep-alive <start/stop/reset>
   ping [-s <packet_size>] [-c <packet_count>] [-W <timeout in sec>] <ipv4/ipv6 address>
   iperf [-s|-c <host>|-a|-h] [options]
   dhcp-stat
#
#
# wlan-version
# wlan-version
   WLAN Driver Version   : v1.3.r46.p1
   WLAN Firmware Version : w8987o-V0, RF878X, FP92, 16.92.21.p88, WPA2_CVE_FIX 1, PVE_FIX 1
# wlan-scan
   Scan scheduled...
# 5 networks found:
   E8:9F:80:9E:16:F8  "Linksys_2g_test" Infra
   mode: 802.11N
   channel: 1
   rssi: -39 dBm
   security: WPA2
   WMM: YES
   802.11K: YES
   802.11W: NA
   WPS: NO
   BC:0F:9A:1F:3C:3C  "KIRAN" Infra
   mode: 802.11N
   channel: 1
   rssi: -87 dBm
   security: WPA2
   WMM: YES
   802.11W: NA
   WPS: NO
   B4:3D:08:54:32:60  "Pratyush" Infra
   mode: 802.11N
   channel: 5
   rssi: -95 dBm
   security: WPA/WPA2 Mixed
   WMM: YES
   802.11W: NA
   WPS: NO
   F8:C4:F3:F6:4F:FA  "Hit" Infra
   mode: 802.11N
   channel: 5
   rssi: -88 dBm
   security: WPA/WPA2 Mixed
   WMM: YES
   802.11K: YES
   802.11V: YES
   802.11W: NA
   WPS: NO
   E8:9F:80:9E:16:F9  "Linksys_5g_test" Infra
   mode: 802.11AC
   channel: 40
   rssi: -39 dBm
   security: OPEN
   WMM: YES
   802.11K: YES
   802.11V: YES
   802.11W: NA
   WPS: NO
#
#
# wlan-add net1 ssid Linksys_5g_test
   Added "net1"
# wlan-conn
# wlan-connect net1
   Connecting to network...
   Use 'wlan-stat' for current connection status.
# Info: ml1: SME: Trying to authenticate with e8:9f:80:9e:16:f9 (SSID='Linksys_5g_test' freq=5200 MHz)
   Info: ml1: Trying to associate with e8:9f:80:9e:16:f9 (SSID='Linksys_5g_test' freq=5200 MHz)
   Info: ml1: Associated with e8:9f:80:9e:16:f9
   app_cb: WLAN: received event 1
   app_cb: WLAN: authenticated to network
   Info: ml1: CTRL-EVENT-CONNECTED - Connection to e8:9f:80:9e:16:f9 completed [id=0 id_str=]
   Info: ml1: CTRL-EVENT-SUBNET-STATUS-UPDATE status=0
   app_cb: WLAN: received event 0
   app_cb: WLAN: connected to network
   Connected to following BSS:
   SSID = [Linksys_5g_test]
   IPv4 Address: [192.168.1.101]
   IPv6 Address: Link-Local   :    FE80::82D2:1DFF:FEE8:2F2F (Preferred)
#
# wlan-info
   Station connected to:
   "net1"
   SSID: Linksys_5g_test
   BSSID: E8:9F:80:9E:16:F9
   mode: 802.11AC
   channel: 40
   role: Infra
   RSSI: -45dBm
   security: none
   IPv4 Address
   address: DHCP
   IP:             192.168.1.101
   gateway:        192.168.1.1
   netmask:        255.255.255.0
   dns1:           192.168.1.1
   dns2:           0.0.0.0
   IPv6 Addresses
   Link-Local   :  FE80::82D2:1DFF:FEE8:2F2F (Preferred)
   uAP not started
# íÿow¿½Þ»_}·¿o½¯½ß¿
#
# ping 192.168.1.100
   PING 192.168.1.100 (192.168.1.100) 56(84) bytes of data
   64 bytes from 192.168.1.100: icmp_req=1 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=2 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=3 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=4 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=5 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=6 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=7 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=8 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=9 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=10 ttl=255 time=2 ms
   --- 192.168.1.101 ping statistics ---
   10 packets transmitted, 10 received, 0% packet loss
#
#
#
# wlan-disc
# wlan-disconnect
# Info: ml1: CTRL-EVENT-DISCONNECTED bssid=e8:9f:80:9e:16:f9 reason=3 locally_generated=1
   Info: ml1: CTRL-EVENT-DSCP-POLICY clear_all
   app_cb: WLAN: received event 10
   app_cb: disconnected
   öí
   command '\0xff' not found
#
#
#
#
# wlan-add net2 ssid Linksys_2g_test wpa2 12345678
   Added "net2"
# wlan-conn
# wlan-connect net2
   Connecting to network...
   Use 'wlan-stat' for current connection status.
# Info: ml1: SME: Trying to authenticate with e8:9f:80:9e:16:f8 (SSID='Linksys_2g_test' freq=2412 MHz)
   Info: ml1: Trying to associate with e8:9f:80:9e:16:f8 (SSID='Linksys_2g_test' freq=2412 MHz)
   Info: ml1: Associated with e8:9f:80:9e:16:f8
   Info: ml1: CTRL-EVENT-SUBNET-STATUS-UPDATE status=0
   Info: RRM: Ignoring radio measurement request: Not associated
   Info: ml1: WPA: Key negotiation completed with e8:9f:80:9e:16:f8 [PTK=CCMP GTK=CCMP]
   Info: ml1: CTRL-EVENT-CONNECTED - Connection to e8:9f:80:9e:16:f8 completed [id=1 id_str=]
   app_cb: WLAN: received event 1
   app_cb: WLAN: authenticated to network
   app_cb: WLAN: received event 0
   app_cb: WLAN: connected to network
   Connected to following BSS:
   SSID = [Linksys_2g_test]
   IPv4 Address: [192.168.1.101]
#
#
# wlan-info
   Station connected to:
   "net2"
   SSID: Linksys_2g_test
   BSSID: E8:9F:80:9E:16:F8
   mode: 802.11N
   channel: 1
   role: Infra
   RSSI: -44dBm
   security: WPA2
   IPv4 Address
   address: DHCP
   IP:             192.168.1.101
   gateway:        192.168.1.1
   netmask:        255.255.255.0
   dns1:           192.168.1.1
   dns2:           0.0.0.0
   IPv6 Addresses
   Link-Local   :  FE80::82D2:1DFF:FEE8:2F2F (Preferred)
   uAP not started
# ping 192.168.1.100
   PING 192.168.1.100 (192.168.1.100) 56(84) bytes of data
   64 bytes from 192.168.1.100: icmp_req=1 ttl=255 time=4 ms
   64 bytes from 192.168.1.100: icmp_req=2 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=3 ttl=255 time=3 ms
   64 bytes from 192.168.1.100: icmp_req=4 ttl=255 time=1 ms
   64 bytes from 192.168.1.100: icmp_req=5 ttl=255 time=1 ms
   64 bytes from 192.168.1.100: icmp_req=6 ttl=255 time=1 ms
   64 bytes from 192.168.1.100: icmp_req=7 ttl=255 time=1 ms
   64 bytes from 192.168.1.100: icmp_req=8 ttl=255 time=2 ms
   64 bytes from 192.168.1.100: icmp_req=9 ttl=255 time=4 ms
   64 bytes from 192.168.1.100: icmp_req=10 ttl=255 time=1 ms
   --- 192.168.1.101 ping statistics ---
   10 packets transmitted, 10 received, 0% packet loss
#
#
#
# wlan-disc
# wlan-disconnect
# Info: ml1: CTRL-EVENT-DISCONNECTED bssid=e8:9f:80:9e:16:f8 reason=3 locally_generated=1
   Info: ml1: CTRL-EVENT-DSCP-POLICY clear_all
   app_cb: WLAN: received event 10
   app_cb: disconnected
#
#
#
#
# wlan-list
   2 networks:
   "net1"
   SSID: Linksys_5g_test
   BSSID: 00:00:00:00:00:00
   mode: 802.11AC
   channel: (Auto)
   role: Infra
   RSSI: -47dBm
   security: none
   IPv6 Addresses
   Link-Local   :  FE80::82D2:1DFF:FEE8:2F2F (Preferred)
   "net2"
   SSID: Linksys_2g_test
   BSSID: 00:00:00:00:00:00
   mode: 802.11N
   channel: (Auto)
   role: Infra
   RSSI: -42dBm
   security: WPA2
   IPv6 Addresses
   Link-Local   :  FE80::82D2:1DFF:FEE8:2F2F (Preferred)
#
