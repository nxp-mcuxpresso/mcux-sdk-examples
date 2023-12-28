Overview
========
The example to demonstrate the usage of Bluetooth BT/BLE profiles.
The example provides a platform for end system designers to quickly evaluate 
EtherMind Bluetooth Software Protocol Stack and Profiles, and implement 
applications using it. The reference applications are included in the kit 
help implementers to quickly develop customized Bluetooth applications to 
typically validate on FreeRTOS.
This document provides step-by-step procedures to build and test the example, 
and also instructions for running the included sample applications.

LE audio roles supported:
1. Unicast Media receiver UMR
2. Broadcast Media sender BMS
3. Broadcat Media receiver BMR

Known issues:
1. Unicast Media sender UMS role works but it is not consistent

Test done:
Tested between Windows binary+Laird USB dongle and RT1170+nRF5340 till NXP firecrest firmware support.
If one setup works as UMS ,other is UMR
If one setup works as BMS ,other is BMR


NOTE.
1. To avoid the potential noise running A2DP source/sink or HFP function. 
APPL_LIMIT_LOGS might need to be enabled in the project setting.
example rebuilding is necessary to make the APPL_LIMIT_LOGS take effect.
2. To make sure the NVM area have clean, all flash sector need be erased before 
download example code.
3. If BT_GAM is enabled then manually give advertise enable option from HCI menu LE menu to enable non GA profiles. Else please follow GA options.
4. HFP will not work with Murata 1ZM module as it has board limitation
5. LEAUDIO macro to be enabled if Audio over LE to be tested. If not enabled BT classic audio is enabled by default.
6. Copy SineWaveMinus48.wav present in bluetooth\tools\resources\a2dp to pendrive/data/a2dp/sample.wav. For LE audio source, A2DP_PL_SOURCE_FS_MEDIA to be enabled in a2dp_pl.c.
7. nRF5340 default public address is all 0.
8. Enable LEAUDIO to test BLE audio feature, BT_THIRDPARTY_SUPPORT for Nordic nrf5340
9. Make sure BOARD_BT_UART_BAUDRATE is 1000000 because Nordic limitation

Connections between RT1170 and nRF5340
PIN NAME |  nRF5340                      |   I.MXRT1170    | PIN NAME OF RT1170 | GPIO NAME OF RT1170
-----------------------------------------------------------------------------------------------------
UART_TXD |  p3 p1.09                     |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
UART_RXD |  P3 p1.08                     |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
UART_CTS |  P3 p1.11                     |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
UART_RTS |  P4 p1.10                     |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
GND      |  P11 b/w two jacks GND pin    |   J26(pin 1)    |    GND             | GND                                                       
------------------------------------------------------------------------------------------------------


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1170 board
- Personal Computer
- One of the following modules:
	- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.
	- Murata uSD-M.2 Adapter (Rev-B2) and Embedded Artists 2EL M.2 Module (Rev-A1)
	- RD Board with SDIO Connector

Hardware re-work for SDIO, UART and PCM lines for EVKB-RT1170:
a. Remove resistors R183 and R1816.
b. Solder 0 ohm resistor to R404, R1901, and R1902.
For PCM(HFP), 
c. Disconnect headers : J79, J80
d. Connect headers : J81, J82
e. Remove Resisters : R1985,R1986,R1987,R1988,R1992,R1993,R1994,R1995
f. Solder 0 ohm resisters : R228, R229, R232, R234, R1903

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use FC (IW61x) RD Board, please change the macro to WIFI_IW61x_BOARD_RD_USD.
If you want to use Murata Type 2EL module with uSD-M2 adapter, please change the macro to WIFI_IW61x_BOARD_MURATA_2EL_USD.
If you want to use Murata Type 2EL M2-A1 module with direct M2 Slot, please change the macro to WIFI_IW61x_BOARD_MURATA_2EL_M2.

Jumper settings for RT1170:
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

NOTE: With direct M2 connection (e.g. 2EL-M2) below connections are not required.

Jumper settings for Murata uSD-M.2 adapter:
  - J12 = 2-3 : M2 VIO set to 3.3v (Blue LED Illuminate)
    J13 = 1-2:  Host VIO set to 3.3v (WLAN-SDIO = 3.3V; and BT-UART & WLAN/BT-CTRL = 3.3V)
  - J1  = 2-3:  3.3V from uSD connector.

The following pins between the evkbmimxrt1170 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM/1XK/2EL Module are connected using male-to-female jumper cables:

----------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT1170    | PIN NAME OF RT1170 | GPIO NAME OF RT1170
----------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
GND				       |  J7(pin 6)  	   |   J26(pin 1)    |    GND		          | GND
----------------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM457-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
UART_RXD |  J10(pin 2)  |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
UART_CTS |  J10(pin 8)  |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
UART_RTS |  J10(pin 6)  |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
GND      |  J6(pin 7)   |   J26(pin 1)    |    GND             | GND
------------------------------------------------------------------------------------

The hardware should be reworked according to the hardware rework guide for evkbmimxrt1170 and AW-CM358-M.2 in document Hardware Rework Guide for EdgeFast BT PAL.

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW4 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.

5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

 1. When the demo starts, the following message about the demo would appear on the Terminal.
       ETHERMIND Menu Application

 2. First, the demo will load the WIFI+BT Module's firmware through the SDIO Interface.
    Once the firmware is successfully Initialized and Loaded
    on the terminal as shown below
For example for RB3P
     Initialize AW-AM457-uSD Driver

 3. After the WIFI+BT module initialization and firmware download, the following demo application menu
    will be shown on the terminal

   0.  Exit. 
   1.  Refresh this Menu. 
 
   2.  EtherMind Init. 
   3.  Bluetooth ON. 
   4.  Bluetooth OFF. 
 
   5.  Set PIN Code. 
   6.  Set Device Role. 
 
   7.  Get Free ACL Tx Queue Buffer. 
   8.  Set LWM/HWM for ACL Tx Queue. 
 
   9.  Configure Service GAP Procedures 
   10. Set Snoop logging. 
 
   11. HCI Operations. 
   12. SDP Operations. 
   13. SDDB Operations. 
   14. RFCOMM Operations. 
   15. MCAP Operations. 
   16. BNEP Operations. 
   17. AVDTP Operations. 
   18. AVCTP Operations. 
   19. OBEX Client Operations. 
   20. OBEX Server Operations. 
   21. L2CAP Operations. 
 
   25. Write Storage event mask. 
 
   30. Security Manager. 
   31. LE SMP. 
   32. LE L2CAP. 
   33. ATT. 
   34. L2CAP ECBFC. 
 
   40. SPP Operations. 
   41. HFP Unit Operations. 
   42. HFP AG Operations. 
   45. DUNP DT Operations. 
   46. DUNP GW Operations. 
   47. SAP Client Operations. 
   48. SAP Server Operations. 
 
   50. OPP Client Operations. 
   51. OPP Server Operations. 
   52. FTP Client Operations. 
   53. FTP Server Operations. 
   54. MAP Client Operations. 
   55. MAP Server Operations. 
   56. PBAP Client Operations. 
   57. PBAP Server Operations. 
   58. CTN Client Operations. 
   59. CTN Server Operations. 
   60. BIP Initiator Operations. 
   61. BIP Responder Operations. 
   62. SYNCP Client Operations. 
   63. SYNCP Server Operations. 
 
   65. A2DP Operations. 
   66. AVRCP Operations. 
   67. HDP Operations. 
   68. PAN Operations. 
 
   70. HID Device Operations. 
   71. HID Host Operations. 
 
   75. DID Client Operations. 
   76. DID Server Operations. 
 
   80. GATT Client Operations. 
   81. GATT Server Operations. 
 
   90. BPP Sender Operations. 
   91. BPP Printer Operations. 
 
  100. Simulate VU. 
 
  200. GA Legacy Options. 
 
  201. GA CCP Client Operations. 
  202. GA CCP Server Operations. 
 
  203. GA MCP Client Operations. 
  204. GA MCP Server Operations. 
 
  205. GA TMAP Operations. 
  206. GA HAP Operations. 
  207. GA BASS Operations.
 
  210. GA Setup. 
 
  220. GA Profile Options.
 
  250. Wake on BLE vendor command. 
  251. H2C sleep vendor command. 
  252. H2C sleep. 
  253. H2C wakeup. 
  254. Host sleep. 
 
  280. Disable Logging.
  281. Enable Logging.
 
  300. BEACON Manager

Your Option ?

 4. Select the option "2(EtherMind Init)" from the above menu to Initialize the EtherMind Module for BT Operations.

 5. After this, wait for above menu to be prompted once again. Also look for the prints
    Initializing EtherMind ...
    EtherMind Stack Successfully Initialized!
    <TODO:> Add the "EtherMind Stack Successfully Initialized!" in the code.

 6. Now, select the option "3(Bluetooth ON)" from the above menu to perform the Bluetooth Initialization process
    which establishes a communication between the EtherMind Host Stack and the WIFI+BT Module.
    The following sequence of messages should be seen on the terminal

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C03 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C01 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C63 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C56 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1009 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
78 50 E5 55 66 70                                  xP.Ufp
-------------------------------------------------------------------

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2002 -> HCI_LE_READ_BUFFER_SIZE_OPCODE
        Command Status: 0x00
        Return Parameters:
-- Dumping 3 Bytes --
-------------------------------------------------------------------
00 01 08                                           ...
-------------------------------------------------------------------

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1005 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 7 Bytes --
-------------------------------------------------------------------
FD 03 3C 07 00 0C 00                               ..<....
-------------------------------------------------------------------

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C13 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C18 -> UNKNOWN
        Command Status: 0x00
Bluetooth ON Initialization Completed.
Stack Version - 020.006.000

 7. After the "Bluetooth ON Initialization Completed" is successfully completed. The Demo application
    automatically sets the BLE Advertisement Parameters, Data and enables Advertisements for a sample
    BLE service. The following prints should be seen on the terminal

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C6D -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2008 -> HCI_LE_SET_ADVERTISING_DATA_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2006 -> HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x200A -> HCI_LE_SET_ADVERTISING_ENABLE_OPCODE
        Command Status: 0x00
Enabled Advertising...
EtherMind Server is now Ready





################################################
TMAP UMR steps
################################################

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?2
choice is 2
Initializing EtherMind ...
EtherMind Stack Successfully Initialized!

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?3
choice is 3
Performing Bluetooth ON ...

Default GAP Role Set to PERIPHERAL
Initialize all required GA modules and then register the database.


   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C03 -> HCI_RESET_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C03 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C01 -> HCI_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C01 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C63 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C63 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C56 -> HCI_WRITE_SIMPLE_PAIRING_MODE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C56 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1009 -> HCI_READ_BD_ADDR_OPCODE
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
16 9C 01 E8 07 C0                                  ......
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1009 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
16 9C 01 E8 07 C0                                  ......
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2060 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2060 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1005 -> HCI_READ_BUFFER_SIZE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1005 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C13 -> HCI_CHANGE_LOCAL_NAME_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C13 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C18 -> HCI_WRITE_PAGE_TIMEOUT_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C18 -> UNKNOWN
        LE Command Status: 0x01

>       Bluetooth ON Initialization Completed.
>       Bluetooth Address: 00:00:00:00:00:00

>       Stack Version - 020.006.000.
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x080F -> HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x080F -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C1A -> HCI_WRITE_SCAN_ENABLE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C1A -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C6D -> UNKNOWN
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C6D -> UNKNOWN
        LE Command Status: 0x01
220
choice is 220


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 2
Registering HCI Callback Function at Location 0
Number of Registered HCI Callback Functions = 1
HCI Callback Registered
Configuring GA
Initializing GA
Registering HCI Callback Function at Location 1
Number of Registered HCI Callback Functions = 2
Retval - GA_SUCCESS (0x0000)
Default settings done for device manager
GA_vc_register. for VOCS
VCP_VOCS Handle: 0x00
Retval - 0x0000
GA_vc_register..for AICS
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2074 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2074 -> HCI_LE_SET_HOST_FEATURE_OPCODE
        LE Command Status: 0x00
VCP_AICS Handle: 0x00
Retval - 0x0000
GA_mc_register_aics
AICS Handle: 0x00
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 1


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 2
GA_tmap_init...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
0
 - Call Gateway(CG): 0x0001
 - Call Terminal(CT): 0x0002
 - Unicast Media Sender (UMS): 0x0004
 - Unicast Media Receiver (UMR): 0x0008
 - Broadcast Media Sender (BMS): 0x0010
 - Broadcast Media Receiver (BMR): 0x0020
Note: If this device supports more than one role, Compute BitMask OR and input
E.g., Call Gateway(CG):0x0001 and Call Terminal(CT):0x0002, Input 0x0003
8
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)
GA_register_audio_capabilities:
Retval - GA_SUCCESS (0x0000)
GA_register_audio_contexts_and_locations:
Retval - GA_SUCCESS (0x0000)
GA_register_audio_sep:
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
1
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 50


    0. Exit.
    1. Refresh this Menu.

    2  Register ASE.
    3. Update ASE QoS Capability
    4. ASE Send Audio Context as Available.
    5. ASE Send Auto Codec Configured.
    6. ASE Send Auto Rx Start Ready for Local ASE Sink.
    7. ASE Send Auto Update Metadata.
    8. ASE Send Auto Disable.
    9. ASE Send Auto Release.
   10. ASE Send Release Complete.

   15. ASE Disconnect CIS.

   20. Display Local ASEs Data for a Connected Device.
   21. Display all Local ASEs Data.
   22. Display all Connected Devices.

Your Option ?: 2
Enter Role: 1 - Sink, 2 - Source:
1
GA_register_audio_sep:
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2  Register ASE.
    3. Update ASE QoS Capability
    4. ASE Send Audio Context as Available.
    5. ASE Send Auto Codec Configured.
    6. ASE Send Auto Rx Start Ready for Local ASE Sink.
    7. ASE Send Auto Update Metadata.
    8. ASE Send Auto Disable.
    9. ASE Send Auto Release.
   10. ASE Send Release Complete.

   15. ASE Disconnect CIS.

   20. Display Local ASEs Data for a Connected Device.
   21. Display all Local ASEs Data.
   22. Display all Connected Devices.

Your Option ?: 0

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 0


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 21
1. Enable 2. Disable
1
Enter the Own Address Type
1. Public Address
2. Random Address
2
Enter the BD Address for Random Type: 112233445566
BD_ADDR Entered 112233445566
BT_hci_le_set_extended_advertising_parameters
Retval - GA_SUCCESS (0x0000)
BT_hci_le_set_extended_advertising_data
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2036 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2036 -> HCI_LE_SET_EXTENDED_ADV_PARAMS_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------


Retval - GA_SUCCESS (0x0000)
BT_hci_le_set_extended_advertising_enable
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2037 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2037 -> HCI_LE_SET_EXTENDED_ADVERTISING_DATA_OPCODE
        LE Command Status: 0x00
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2039 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2039 -> HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_OPCODE
        LE Command Status: 0x00

[ATT]:[0x09: 0x05]: Received ATT Event 0x81 with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
31 12 63 E8 07 C0 00                               1.c....
-------------------------------------------------------------------

Received ATT Connection Indication, Result 0x0000!
[ATT] ATT connected over LE Fixed Channel!
[ATT HANDLE]: (Dev ID: 0x09, ATT ID: 0x05

[IPSPR]: ACL Connected with IPSP Node

[CPMC]: ACL Connected with CPM Sensor
[ATT]:[0x09]:[0x05]: Received event 0x81 with result 0x0000
Received ATT Connection Indication with result 0x0000

Received ATT_CONNECTION_IND on LE for ATT Handle:
   -> Device_ID       : 0x09
   -> ATT_Instance_ID : 0x05

[BASIC]: In appl_basic_connect
Subevent : HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT.
status = 0x00
connection_handle = 0x0000
role = 0x01
appl_peer_addr_type = 0x00
appl_peer_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
31 12 63 E8 07 C0                                  1.c...
-------------------------------------------------------------------

local_resolvable_pvt_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 00 00 00 00 00                                  ......
-------------------------------------------------------------------

peer_resolvable_pvt_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 00 00 00 00 00                                  ......
-------------------------------------------------------------------

conn_interval = 0x0050
conn_latency = 0x0000
supervision_timeout = 0x09D0
clock_accuracy = 0x07
Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
Connection Handle: 0x0000
Channel Selection Algorithm: 0x01
Subevent : HCI_LE_ADVERTISING_SET_TERMINATED_SUBEVENT.
status = 0x00
Advertising Handle: 0x01
Connection Handle: 0x0000
Num_Completed_Extended_Advertising_Events: 0xCE
[ATT]:[0x09: 0x05]: Received ATT Event 0x02 with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
40 00                                              @.
-------------------------------------------------------------------

Received Exchange MTU Request with Result 0x0000. MTU Size = 0x0040!
[ATT]:[0x09]:[0x05]: Received event 0x02 with result 0x0000
Received Exchange MTU Request with MTU Size = 0x0040!
Sent Response with retval 0x0000

[BASIC]: Updated MTU is 64 for Appl Handle 0x05
[ATT]:[0x09: 0x05]: Received ATT Event 0x03 with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
40 00                                              @.
-------------------------------------------------------------------

Received Exchange MTU Response with Result 0x0000. MTU Size = 0x0040!

Event   : SMP_LONG_TERM_KEY_REQUEST
BD Address : 31 12 63 E8 07 C0
BD addr type : Public Address
Div  : 0x0000
Rand : 00 00 00 00 00 00 00 00


Sending +ve LTK request reply.
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x201A -> HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_OPCODE
        Command Status: 0x00
        Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
00 00                                              ..
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x201A -> HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
00 00                                              ..
-------------------------------------------------------------------



Recvd SMP_AUTHENTICATION_COMPLETE
BD Address : 31 12 63 E8 07 C0
BD addr type : Public Address
Status : 0000

Confirmed Authentication using Encryption
[BASIC]: In appl_basic_connect
Received HCI_ENCRYPTION_CHANGE_EVENT.
        Status: 0x00
 -> HCI_Encryption Status Succeeded
        Connection Handle: 0x0000
        Encryption Enable: 0x01 -> Encryption ON

LE: Unknown Event Code 0x08 Received.
3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: [ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
01 00 FF FF 00 28 55 18                            .....(U.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0001-0xFFFF
[APPL]: Number of occurrences of UUID 0x0000 = 0x0001
[APPL]: <<< Handling Find By Type Value request, Range 0x0073-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
73 00 FF FF 00 28 55 18                            s....(U.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0073-0xFFFF
[APPL]: Sent Error Response with result 0x0000
[APPL]: <<< Handling Find By Type Value request, Range 0x0073-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x08 with result 0x0000

-- Dumping 6 Bytes --
-------------------------------------------------------------------
70 00 72 00 03 28                                  p.r..(
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x08 with result 0x0000
[APPL]: >>> Handling Read By Type request, Range 0x0070-0x0072
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0071 SID:0x0B CID:0x0027
[APPL]:[0x0000]: Handle 0x0071 -> Value Len 0x0005
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Read By Type Response with result 0x0000
[APPL]: <<< Handling Read By Type request, Range 0x0070-0x0072
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
72 00                                              r.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x0072 SID:0x0B CID:0x0027
1


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 10
Device:
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
Connection Handle: 0x0000
Initiator: 0x00
Index: 0x01

Enter the Conn device index: 1
GA_tmap_discover...
Retval - GA_SUCCESS (0x0000)
appl_tmap_handle:0x0000

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? [ATT]:[0x09: 0x05]: Received ATT Event 0xF0 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
55                                                 U
-------------------------------------------------------------------

Received GATT_PS_DISCOVERY_RSP
No. Primary Services - 1

UUID: 0x1855 (Unknown)
Start Hdl: 0x0070, End Hdl: 0x0072

[ATT]:[0x09: 0x05]: Received ATT Event 0xF3 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
71                                                 q
-------------------------------------------------------------------

Received GATT_CHAR_DISCOVERY_RSP
No. Characteristics - 1

(Unknown)
Char Handle: 0x0071, UUID: 0x2B51
Property: 0x02, Value Handle: 0x0072

[ATT]:[0x09: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
04 00                                              ..
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 2 Bytes --
-------------------------------------------------------------------
04 00                                              ..
-------------------------------------------------------------------

************************** CALLBACK: GA TMAP Client **************************
[Profile]     : TMAP (0x1855)
[SECTION]     : TMAP CLIENT EVENTS
[SUB-SECTION] : TMAP CLIENT-DISCOVER
[TYPE]        : DISCOVER RESPONSE
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : TMAP_DISCOVER_CNF (0x01)
[Service Cntx]: 0xCC

-----------------------------------------------------------------
Data Length: 0x02
Data:
Len: 0x02,  Role(s): 0x0004
        -Unicast Media Sender (UMS) (0x0004)
*********************************************************************
[ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
01 00 FF FF 00 28 50 18                            .....(P.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0001-0xFFFF
[APPL]: Number of occurrences of UUID 0x0000 = 0x0001
[APPL]: <<< Handling Find By Type Value request, Range 0x0080-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
80 00 FF FF 00 28 50 18                            .....(P.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0080-0xFFFF
[APPL]: Sent Error Response with result 0x0000
[APPL]: <<< Handling Find By Type Value request, Range 0x0080-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x08 with result 0x0000

-- Dumping 6 Bytes --
-------------------------------------------------------------------
73 00 7F 00 03 28                                  s....(
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x08 with result 0x0000
[APPL]: >>> Handling Read By Type request, Range 0x0073-0x007F
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0074 SID:0x0C CID:0x0028
[APPL]:[0x0000]: Handle 0x0074 -> Value Len 0x0005
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0077 SID:0x0C CID:0x0029
[APPL]:[0x0001]: Handle 0x0077 -> Value Len 0x0005
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x007A SID:0x0C CID:0x002A
[APPL]:[0x0002]: Handle 0x007A -> Value Len 0x0005
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x007D SID:0x0C CID:0x002B
[APPL]:[0x0003]: Handle 0x007D -> Value Len 0x0005
[APPL]:Search Complete, #Handle UUID List found = 0x04
[APPL]: Sent Read By Type Response with result 0x0000
[APPL]: <<< Handling Read By Type request, Range 0x007D-0x007F
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
76 00 76 00                                        v.v.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x0076-0x0076
[APPL]:[0x0000]: Handle 0x0076 -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x0076-0x0076
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
79 00 79 00                                        y.y.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x0079-0x0079
[APPL]:[0x0000]: Handle 0x0079 -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x0079-0x0079
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
7C 00 7C 00                                        |.|.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x007C-0x007C
[APPL]:[0x0000]: Handle 0x007C -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x007C-0x007C
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
7F 00 7F 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x007F-0x007F
[APPL]:[0x0000]: Handle 0x007F -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x007F-0x007F
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
7C 00 01 00                                        |...
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x007C of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x007C SID:0x0C CID:0x002A
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
7F 00 01 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x007F of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x007F SID:0x0C CID:0x002B
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
79 00 01 00                                        y...
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x0079 of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x0079 SID:0x0C CID:0x0029
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
76 00 01 00                                        v...
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x0076 of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x0076 SID:0x0C CID:0x0028
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
7B 00                                              {.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x007B SID:0x0C CID:0x002A
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
7E 00                                              ~.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x007E SID:0x0C CID:0x002B
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
75 00                                              u.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x0075 SID:0x0C CID:0x0028
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
78 00                                              x.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x0078 SID:0x0C CID:0x0029
[ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
01 00 FF FF 00 28 4E 18                            .....(N.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0001-0xFFFF
[APPL]: Number of occurrences of UUID 0x0000 = 0x0001
[APPL]: <<< Handling Find By Type Value request, Range 0x0001-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
8A 00 FF FF 00 28 4E 18                            .....(N.
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x008A-0xFFFF
[APPL]: Sent Error Response with result 0x0000
[APPL]: <<< Handling Find By Type Value request, Range 0x008A-0xFFFF, with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x08 with result 0x0000

-- Dumping 6 Bytes --
-------------------------------------------------------------------
80 00 89 00 03 28                                  .....(
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x08 with result 0x0000
[APPL]: >>> Handling Read By Type request, Range 0x0080-0x0089
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0081 SID:0x0D CID:0x002C
[APPL]:[0x0000]: Handle 0x0081 -> Value Len 0x0005
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0084 SID:0x0D CID:0x002D
[APPL]:[0x0001]: Handle 0x0084 -> Value Len 0x0005
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x09 Handle:0x0087 SID:0x0D CID:0x002E
[APPL]:[0x0002]: Handle 0x0087 -> Value Len 0x0005
[APPL]:Search Complete, #Handle UUID List found = 0x03
[APPL]: Sent Read By Type Response with result 0x0000
[APPL]: <<< Handling Read By Type request, Range 0x0087-0x0089
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
83 00 83 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x0083-0x0083
[APPL]:[0x0000]: Handle 0x0083 -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x0083-0x0083
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
86 00 86 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x0086-0x0086
[APPL]:[0x0000]: Handle 0x0086 -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x0086-0x0086
[ATT]:[0x09: 0x05]: Received ATT Event 0x04 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
89 00 89 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x04 with result 0x0000
Received Find Info Request with result 0x0000
[APPL]: >>> Handling Find Info request, Range 0x0089-0x0089
[APPL]:[0x0000]: Handle 0x0089 -> UUID Len 0x0002
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Find Info Response with result 0x0000
[APPL]: <<< Handling Find Info request, Range 0x0089-0x0089
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
83 00 01 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x0083 of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x0083 SID:0x0D CID:0x002C
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
86 00 01 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x0086 of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x0086 SID:0x0D CID:0x002D
[ATT]:[0x09: 0x05]: Received ATT Event 0x12 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
89 00 01 00                                        ....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x12 with result 0x0000
[APPL] Write Request from Peer for handle 0x0089 of Length 2

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x92 from DEVID:0x09 Handle:0x0089 SID:0x0D CID:0x002E
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
85 00                                              ..
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x0085 SID:0x0D CID:0x002D
[ATT]:[0x09: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
88 00                                              ..
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x09 Handle:0x0088 SID:0x0D CID:0x002E
[ATT]:[0x09: 0x05]: Received ATT Event 0x52 with result 0x0000

-- Dumping 32 Bytes --
-------------------------------------------------------------------
82 00 01 01 01 02 02 06 00 00 00 00 13 02 01 08    ................
02 02 01 05 03 01 00 00 00 03 04 64 00 02 05 01    ...........d....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x52 with result 0x0000
[APPL] Write Request from Peer for handle 0x0082 of Length 30

-- Dumping 30 Bytes --
-------------------------------------------------------------------
01 01 01 02 02 06 00 00 00 00 13 02 01 08 02 02    ................
01 05 03 01 00 00 00 03 04 64 00 02 05 01          .........d....
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x84 from DEVID:0x09 Handle:0x0082 SID:0x0D CID:0x002C
************************** CALLBACK: GA Server **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS SERVER EVENTS
[SUB-SECTION] : ASCS SERVER-WRITE
[TYPE]        : WRITE REQUEST
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_CONFIGURE_IND (0x11)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
RECEIVED DATA
-----------------------------------------------------------------
Data Length: 0x08
Data:
Len: 0x01,  ASE ID: 0x01
Len: 0x01,  Role: Sink(0x01)
Len: 0x01,  Target_Latency: Target balanced latency and reliability (0x02)
Len: 0x01,  Target_PHY:
                - LE 2M PHY preferred
Len: 0x05,  Codec ID:
                - Len: 0x01,  Coding Format: LC3 (0x06),
                - Len: 0x02,  Company ID: (0x0000)
                - Len: 0x02,  Vendor Specific Codec ID: (0x0000)
Len: 0x01,  Codec_Specific_Configuration_Length: 19
Len: 0x18,  Codec_Specific_Configuration:
                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x01
                - Len: 0x01,  Value: Sampling Frequencies: 48000 Hz (0x0008)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x02
                - Len: 0x01,  Value: Frame Durations: 10 ms (0x01)

                - Len: 0x01,  Length: 0x05
                - Len: 0x01,  Type: 0x03
                - Len: 0x04,  Value: Audio Channel Allocation:
                - Front Left (0x1)

                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x04
                - Len: 0x02,  Value: Octets per Codec Frame: 0x0064

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: Codec Frame Blocks Per SDU: 0x01

*********************************************************************
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
[APPL] Received Write Command for Handle 0x0082 of Length 30

82 00 01 01 01 00 00                               .......
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
-- Dumping 30 Bytes --
-------------------------------------------------------------------
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 46 Bytes --
-------------------------------------------------------------------
01 01 01 02 02 06 00 00 00 00 13 02 01 08 02 02    ................
85 00 01 01 00 02 0D 5F 00 40 9C 00 40 9C 00 00    ......._.@..@...
01 05 03 01 00 00 00 03 04 64 00 02 05 01          .........d....
-------------------------------------------------------------------

00 00 00 00 00 06 00 00 00 00 13 02 01 08 02 02    ................
01 05 03 01 00 00 00 03 04 64 00 02 05 01          .........d....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x52 with result 0x0000

-- Dumping 20 Bytes --
-------------------------------------------------------------------
82 00 02 01 01 01 01 10 27 00 00 02 64 00 0D 5F    ........'...d.._
00 40 9C 00                                        .@..
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x52 with result 0x0000
[APPL] Write Request from Peer for handle 0x0082 of Length 18

-- Dumping 18 Bytes --
-------------------------------------------------------------------
02 01 01 01 01 10 27 00 00 02 64 00 0D 5F 00 40    ......'...d.._.@
9C 00                                              ..
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x84 from DEVID:0x09 Handle:0x0082 SID:0x0D CID:0x002C
************************** CALLBACK: GA Server **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS SERVER EVENTS
[SUB-SECTION] : ASCS SERVER-WRITE
[TYPE]        : WRITE REQUEST
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_SETUP_IND (0x12)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
RECEIVED DATA
-----------------------------------------------------------------
Data Length: 0x08
Data:
Len: 0x01,  ASE ID: 0x01
Len: 0x01,  CIG_ID - 0x01
Len: 0x01,  CIS_ID - 0x01
Len: 0x04,  SDU_Interval: 10000(0x00002710)
Len: 0x01,  Framing: Unframed (0x00)
Len: 0x01,  PHY:
                - LE 2M PHY preferred
Len: 0x02,  Max_SDU: 100(0x0064)
Len: 0x01,  Retransmission_Number: (13)0x0D
Len: 0x02,  Max_Transport_Latency: 95(0x005F)
Len: 0x04,  Presentation_Delay: 40000(0x00009C40)
*********************************************************************
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
82 00 02 01 01 00 00                               .......
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
[APPL] Received Write Command for Handle 0x0082 of Length 18

-- Dumping 18 Bytes --
-------------------------------------------------------------------
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 19 Bytes --
-------------------------------------------------------------------
02 01 01 01 01 10 27 00 00 02 64 00 0D 5F 00 40    ......'...d.._.@
85 00 01 02 01 01 10 27 00 00 02 64 00 0D 5F 00    .......'...d.._.
9C 00                                              ..
-------------------------------------------------------------------

40 9C 00                                           @..
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
[ATT]:[0x09: 0x05]: Received ATT Event 0x52 with result 0x0000

-- Dumping 13 Bytes --
-------------------------------------------------------------------
82 00 03 01 01 07 03 02 04 00 02 05 01             .............
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x52 with result 0x0000
[APPL] Write Request from Peer for handle 0x0082 of Length 11

-- Dumping 11 Bytes --
-------------------------------------------------------------------
03 01 01 07 03 02 04 00 02 05 01                   ...........
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x84 from DEVID:0x09 Handle:0x0082 SID:0x0D CID:0x002C
************************** CALLBACK: GA Server **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS SERVER EVENTS
[SUB-SECTION] : ASCS SERVER-WRITE
[TYPE]        : WRITE REQUEST
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_START_IND (0x13)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
RECEIVED DATA
-----------------------------------------------------------------
Data Length: 0x08
Data:
Len: 0x01,  ASE ID: 0x01
Len: 0x01,  Metadata_Length - 0x07
Len: 0x07,  Metadata -
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x02
                - Len: 0x02,  Value: Streaming Audio Contexts:  - Media (0x0004)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: CCID List: 0x1

*********************************************************************
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
[APPL] Received Write Command for Handle 0x0082 of Length 11

-- Dumping 11 Bytes --
-------------------------------------------------------------------
82 00 03 01 01 00 00                               .......
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
03 01 01 07 03 02 04 00 02 05 01                   ...........
-------------------------------------------------------------------

[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 14 Bytes --
-------------------------------------------------------------------
85 00 01 03 01 01 07 03 02 04 00 02 05 01          ..............
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
Subevent : HCI_LE_CIS_REQUEST_SUBEVENT.
ACL Connection Handle: 0x0000
CIS Connection Handle: 0x0002
CIG_ID: 0x01
CIS_ID: 0x01
Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2066 (UNKNOWN)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2066 (HCI_LE_ACCEPT_CIS_REQUEST_OPCODE)
Subevent : HCI_LE_CIS_ESTABLISHED_SUBEVENT.
Status: 0x00
Connection Handle: 0x0002
CIG_Sync_Delay: 0x0000100E
CIS_Sync_Delay: 0x0000100E
Transport_Latency_M_To_S: 0x00000005
Transport_Latency_S_To_M: 0x00000005
PHY_M_To_S: 0x02
PHY_S_To_M: 0x02
[APPL][GA][UCS][SNK]: ISO Data Path Setup Status: In-progress
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x206E -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x206E -> HCI_LE_SETUP_ISO_DATA_PATH_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------


[APPL][GA][UCS][SNK]: ISO Data Path Setup Status: Complete
1

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 50


    0. Exit.
    1. Refresh this Menu.

    2  Register ASE.
    3. Update ASE QoS Capability
    4. ASE Send Audio Context as Available.
    5. ASE Send Auto Codec Configured.
    6. ASE Send Auto Rx Start Ready for Local ASE Sink.
    7. ASE Send Auto Update Metadata.
    8. ASE Send Auto Disable.
    9. ASE Send Auto Release.
   10. ASE Send Release Complete.

   15. ASE Disconnect CIS.

   20. Display Local ASEs Data for a Connected Device.
   21. Display all Local ASEs Data.
   22. Display all Connected Devices.

Your Option ?: 6
Device:
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
Connection Handle: 0x0000
Initiator: 0x00
Index: 0x01

Enter the Conn device index: 1
Enter ASE Count: 1
Enter ASE ID:
1
GA_notify_ase_receiver_start_ready
Retval - GA_SUCCESS (0x0000)
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 14 Bytes --
-------------------------------------------------------------------
[APPL][GA][UCS][SNK]: LC3 Decoder Setup Status: Created
[APPL][GA][UCS][SNK]: Audio PL Playback Setup Status: Success
85 00 01 04 01 01 07 03 02 04 00 02 05 01          ..............
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
[APPL][GA][UCS][SNK]: Audio PL Playback Start Process Status: Success


    0. Exit.
    1. Refresh this Menu.

    2  Register ASE.
    3. Update ASE QoS Capability
    4. ASE Send Audio Context as Available.
    5. ASE Send Auto Codec Configured.
    6. ASE Send Auto Rx Start Ready for Local ASE Sink.
    7. ASE Send Auto Update Metadata.
    8. ASE Send Auto Disable.
    9. ASE Send Auto Release.
   10. ASE Send Release Complete.

   15. ASE Disconnect CIS.

   20. Display Local ASEs Data for a Connected Device.
   21. Display all Local ASEs Data.
   22. Display all Connected Devices.

Your Option ?: [ATT]:[0x09: 0x05]: Received ATT Event 0x52 with result 0x0000

-- Dumping 5 Bytes --
-------------------------------------------------------------------
82 00 05 01 01                                     .....
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x52 with result 0x0000
[APPL] Write Request from Peer for handle 0x0082 of Length 3

-- Dumping 3 Bytes --
-------------------------------------------------------------------
05 01 01                                           ...
-------------------------------------------------------------------

Received Requests from Peer on the Server Side with Op:0x84 from DEVID:0x09 Handle:0x0082 SID:0x0D CID:0x002C
************************** CALLBACK: GA Server **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS SERVER EVENTS
[SUB-SECTION] : ASCS SERVER-WRITE
[TYPE]        : WRITE REQUEST
[BD ADDRESS]  : 0x31 0x12 0x63 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_SUSPEND_IND (0x14)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
RECEIVED DATA
-----------------------------------------------------------------
Data Length: 0x08
Data:
Len: 0x01,  ASE ID: 0x01
[APPL][GA][UCS][SNK]: LC3 Decoder Setup Status: Deleted
[APPL][GA][UCS][SNK]: Audio PL Playback Stop Process Status: Success
*********************************************************************
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
[APPL] Received Write Command for Handle 0x0082 of Length 3
82 00 05 01 01 00 00                               .......
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000

-- Dumping 3 Bytes --
[ATT]:[0x09: 0x05]: Received ATT Event 0x1C with result 0x0000

-- Dumping 19 Bytes --
-------------------------------------------------------------------
-------------------------------------------------------------------
85 00 01 02 01 01 10 27 00 00 02 64 00 0D 5F 00    .......'...d.._.
05 01 01                                           ...
-------------------------------------------------------------------

40 9C 00                                           @..
-------------------------------------------------------------------

[ATT]:[0x09]:[0x05]: Received event 0x1C with result 0x0000
################################
TMAP UMR
################################



################################
TMAP UMS steps
################################
################################
TMAP UMS
################################



################################
TMAP BMS steps
################################
   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?2
choice is 2
Initializing EtherMind ...
EtherMind Stack Successfully Initialized!

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?3
choice is 3
Performing Bluetooth ON ...

Default GAP Role Set to PERIPHERAL
Initialize all required GA modules and then register the database.


   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C03 -> HCI_RESET_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C03 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C01 -> HCI_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C01 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C63 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C63 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C56 -> HCI_WRITE_SIMPLE_PAIRING_MODE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C56 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1009 -> HCI_READ_BD_ADDR_OPCODE
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
D4 BD 27 E8 07 C0                                  ..'...
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1009 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
D4 BD 27 E8 07 C0                                  ..'...
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2060 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2060 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1005 -> HCI_READ_BUFFER_SIZE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1005 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C13 -> HCI_CHANGE_LOCAL_NAME_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C13 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C18 -> HCI_WRITE_PAGE_TIMEOUT_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C18 -> UNKNOWN
        LE Command Status: 0x01

>       Bluetooth ON Initialization Completed.
>       Bluetooth Address: 00:00:00:00:00:00

>       Stack Version - 020.006.000.
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x080F -> HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x080F -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C1A -> HCI_WRITE_SCAN_ENABLE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C1A -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C6D -> UNKNOWN
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C6D -> UNKNOWN
        LE Command Status: 0x01
220
choice is 220


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 2
Registering HCI Callback Function at Location 0
Number of Registered HCI Callback Functions = 1
HCI Callback Registered
Configuring GA
Initializing GA
Registering HCI Callback Function at Location 1
Number of Registered HCI Callback Functions = 2
Retval - GA_SUCCESS (0x0000)
Default settings done for device manager
GA_vc_register. for VOCS
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2074 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2074 -> HCI_LE_SET_HOST_FEATURE_OPCODE
        LE Command Status: 0x00
VCP_VOCS Handle: 0x00
Retval - 0x0000
GA_vc_register..for AICS
VCP_AICS Handle: 0x00
Retval - 0x0000
GA_mc_register_aics
AICS Handle: 0x00
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 2
GA_tmap_init...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
0
 - Call Gateway(CG): 0x0001
 - Call Terminal(CT): 0x0002
 - Unicast Media Sender (UMS): 0x0004
 - Unicast Media Receiver (UMR): 0x0008
 - Broadcast Media Sender (BMS): 0x0010
 - Broadcast Media Receiver (BMR): 0x0020
Note: If this device supports more than one role, Compute BitMask OR and input
E.g., Call Gateway(CG):0x0001 and Call Terminal(CT):0x0002, Input 0x0003
10
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)
Enter no. of Subgroup: 1
Enter no. of BIS: 1
Enter Audio Locations (0x):
Not Allowed:                 0x00000000
Front Left:                  0x00000001
Front Right:                 0x00000002
Front Center:                0x00000004
Low Frequency Effects 1:     0x00000008
Back Left:                   0x00000010
Back Right:                  0x00000020
Front Left of Center:        0x00000040
Front Right of Center:       0x00000080
Back Center:                 0x00000100
Low Frequency Effects 2:     0x00000200
Side Left:                   0x00000400
Side Right:                  0x00000800
Top Front Left:              0x00001000
Top Front Right:             0x00002000
Top Front Center:            0x00004000
Top Center:                  0x00008000
Top Back Left:               0x00010000
Top Back Right:              0x00020000
Top Side Left:               0x00040000
Top Side Right:              0x00080000
Top Back Center:             0x00100000
Bottom Front Center:         0x00200000
Bottom Front Left:           0x00400000
Bottom Front Right:          0x00800000
Front Left Wide:             0x01000000
Front Right Wide:            0x02000000
Left Surround:               0x04000000
Right Surround:              0x08000000
RFU 1:                       0x10000000
RFU 2:                       0x20000000
RFU 3:                       0x40000000
RFU 4:                       0x80000000
TMAP Role: UMS, M: Front Left, Front Right
TMAP Role: UMR, M: Front Left, Front Right, Front Right and Front Left
TMAP Role: BMS, M: Front Left, Front Right
TMAP Role: BMR, M: Front Left or Front Right or Front Right and Front Left
Note: If this PACS/ASE/BIG needs to transmit more than 1 channel LC3 Frame, Compute BitMask OR and input
Front Left(0x00000001) and Side Left(0x00000400): 401
Note: For PACS each Location added here will mandate that thereshall be atleast one ASE registered representing this Location
OR
There shall be a single ASE which supports all the Audio Location
Note: For ASE each Location added here will mandate that the PACS hasregistered this Audio Location
BIS will not transmit more than 1 channel LC3 Frame.Do not compute BitMask OR during input
1
**************Broadcast Audio Stream Configuration**************
Packing: Sequential (0x00)
Encryption: 0x00
SDU Interval: 0x002710(10000us)
Framing: Unframed (0x00)
PHY Preference:
                - LE 2M PHY preferred
MAX SDU: 0x0064
Retransmission Number: 0x04
Max Transport Latency: 0x0014(20ms)
Presentation Delay: 0x009C40(40000us)
Broadcast Code: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
Broadcast ID: 0xDEADBE
Number of Subgroups: 0x01
Subgroup Info [1]:
No. of BIS: 0x01
Len: 0x05,  Codec ID:
                - Len: 0x01,  Coding Format: LC3 (0x06),
                - Len: 0x02,  Company ID: (0x0000)
                - Len: 0x02,  Vendor Specific Codec ID: (0x0000)
Len: 0x01,  Codec_Specific_Configuration_Length: 0x13
Len: 0x13,  Codec_Specific_Configuration:
                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x01
                - Len: 0x01,  Value: Sampling Frequencies: 48000 Hz (0x0008)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x02
                - Len: 0x01,  Value: Frame Durations: 10 ms (0x01)

                - Len: 0x01,  Length: 0x05
                - Len: 0x01,  Type: 0x03
                - Len: 0x04,  Value: Audio Channel Allocation:
                - Front Left (0x1)

                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x04
                - Len: 0x02,  Value: Octets per Codec Frame: 0x0064

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: Codec Frame Blocks Per SDU: 0x01

Len: 0x01,  Metadata_Length: 0x09
Len: 0x09,  Metadata:
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x02
                - Len: 0x02,  Value: Streaming Audio Contexts:  - Media (0x0004)

                - Len: 0x01,  Length: 0x04
                - Len: 0x01,  Type: 0x04
                - Len: 0x03,  Value: Language: eng

BIS Info [1]:
Len: 0x01,  Codec_Specific_Configuration_Length: 0x06
Len: 0x06,  Codec_Specific_Configuration:
                - Len: 0x01,  Length: 0x05
                - Len: 0x01,  Type: 0x03
                - Len: 0x04,  Value: Audio Channel Allocation:
                - Front Left (0x1)

****************************************************************

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 60


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 40
GA_broadcast_alloc_session
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 42
Select your choice:
1. UnEncrypted Stream
2. Encrypted Stream
1
Configuring unencrypted broadcast
GA_broadcast_configure_session
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 43
GA_broadcast_register_sep
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 44
GA_broadcast_register_advset
Retval - GA_SUCCESS (0x0000)
GA_broadcast_setup_announcement
Retval - GA_SUCCESS (0x0000)

Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2036 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2036 -> HCI_LE_SET_EXTENDED_ADV_PARAMS_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------



    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2037 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2037 -> HCI_LE_SET_EXTENDED_ADVERTISING_DATA_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2039 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2039 -> HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x203E -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x203E -> HCI_LE_SET_PERIODIC_ADV_PARAMS_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x203F -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x203F -> HCI_LE_SET_PERIODIC_ADVERTISING_DATA_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2040 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2040 -> HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_OPCODE
        LE Command Status: 0x00
************************** CALLBACK: GA BC Source **************************
[Profile]     : CAP
[SECTION]     : BCAS SOURCE EVENTS
[SUB-SECTION] : BCAS SOURCE-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SETUP_ANNOUNCEMENT_CNF (0xC0)

-----------------------------------------------------------------
Data Length: 0x00
Data: NULL
*********************************************************************
1


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 46
GA_broadcast_start
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2068 (UNKNOWN)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2068 (HCI_LE_CREATE_BIG_OPCODE)
Subevent : HCI_LE_CREATE_BIG_COMPLETE_SUBEVENT.
Status: 0x00
BIG_Handle: 0x00
BIG_Sync_Delay: 0x00000660
Transport_Latency_BIG: 0x00000660
PHY: 0x02
NSE: 0x03
BN: 0x01
PTO: 0x00
IRC: 0x03
Max_PDU: 0x0064
ISO_Inerval: 0x0008
Num_BIS: 0x01
Connection Handles of BISes:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
06 00                                              ..
-------------------------------------------------------------------

************************** CALLBACK: GA BC Source **************************
[Profile]     : CAP
[SECTION]     : BCAS SOURCE EVENTS
[SUB-SECTION] : BCAS SOURCE-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_START_CNF (0xC2)

-----------------------------------------------------------------
Data Length: 0x14
Data:
Len: 0x01,  Error Code: 0x00
Len: 0x01,  BIG Handle: 0x00
Len: 0x04,  BIG Sync Delay: 0x00000660
Len: 0x04,  Transport Latency BIG: 0x00000660
Len: 0x01,  Phy: 0x02
Len: 0x01,  NSE: 0x03
Len: 0x01,  BN: 0x01
Len: 0x01,  PTO: 0x00
Len: 0x01,  IRC: 0x03
Len: 0x02,  Max PDU: 0x0064
Len: 0x02,  ISO Interval: 0x0008
Len: 0x01,  Number of BISes: 0x01
BIS[1]:
        Len: 0x02,  BIS Conn Handle: 0x0006
[APPL][GA][BC_SRC]: ISO Data Path Setup Status: In-progress
*********************************************************************
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x206E -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
06 00                                              ..
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x206E -> HCI_LE_SETUP_ISO_DATA_PATH_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
06 00                                              ..
-------------------------------------------------------------------


HCI_COMMAND_COMPLETE_EVENT
[APPL][GA][BC_SRC]: ISO Data Path Setup Status: Complete
[APPL][GA][BC_SRC]: LC3 Encoder Setup Status: Created
[APPL][GA][BC_SRC]: Audio PL Generator Setup Status: Success
[APPL][GA][BC_SRC]: Audio PL Generator Start Process Status: Success
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 1* * * * * * * * * * * * * * * * * * * * * * *


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 4* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 7* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
GA_broadcast_suspend
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x206A (UNKNOWN)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x206A (HCI_LE_TERMINATE_BIG_OPCODE)
* * * * * * * Subevent : HCI_LE_TERMINATE_BIG_COMPLETE_SUBEVENT.
BIG_Handle: 0x00
Reason: 0x16
************************** CALLBACK: GA BC Source **************************
[Profile]     : CAP
[SECTION]     : BCAS SOURCE EVENTS
[SUB-SECTION] : BCAS SOURCE-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SUSPEND_CNF (0xC3)

-----------------------------------------------------------------
Data Length: 0x02
Data:
Len: 0x01,  BIG Handle: 0x00
Len: 0x01,  Reason: 0x16
[APPL][GA][BC_SRC]: ISO Data Path Removal Status: Complete
[APPL][GA][BC_SRC]: LC3 Encoder Setup Status: Deleted
! [APPL][GA][BC_SRC]: Audio PL Generator Stop Process Status: Success
*********************************************************************
1


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 45
GA_broadcast_end_announcement
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. BA Init.
    3. BA Shutdown.

   10. Scan for Scan Delegator
   11. Stop Scan for Scan Delegator
   12. Connect to Scan Delegator

   20. Setup BASS session with Peer Device
   21. Read all BASS Instances
   22. Add the Broadcast Source
   23. Modify the Broadcast Source
   24. Set Broadcast Code
   25. Remove the Source
   26. Release BASS

   30. Scan for Broadcast Source.
   31. Stop Scan for Broadcast Source.
   32. Associate to the Broadcast Source.
   33. Send Sync Info to BA SD.
   34. Dissociate from the Broadcast Source.


   40. Allocate Session.
   41. Free Session.
   42. Configure Session.
   43. Register Source EndPoint.
   44. Setup Announcement.
   45. End Announcement.
   46. Start Broadcast.
   47. Suspend Broadcast.

   80. Get Context Info.
   81. Set Context Info.

   82. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2040 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2040 -> HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_OPCODE
        LE Command Status: 0x00
************************** CALLBACK: GA BC Source **************************
[Profile]     : CAP
[SECTION]     : BCAS SOURCE EVENTS
[SUB-SECTION] : BCAS SOURCE-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_END_ANNOUNCEMENT_CNF (0xC1)

-----------------------------------------------------------------
Data Length: 0x00
Data: NULL
*********************************************************************
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2039 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2039 -> HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_OPCODE
        LE Command Status: 0x00



################################
TMAP BMS
################################




################################
TMAP BMR steps
################################
       EtherMind Menu Application
     Initialize Firecrest Driver

 
   0.  Exit. 
   1.  Refresh this Menu. 
 
   2.  EtherMind Init. 
   3.  Bluetooth ON. 
   4.  Bluetooth OFF. 
 
   5.  Set PIN Code. 
   6.  Set Device Role. 
 
   7.  Get Free ACL Tx Queue Buffer. 
   8.  Set LWM/HWM for ACL Tx Queue. 
 
   9.  Configure Service GAP Procedures 
   10. Set Snoop logging. 
 
   11. HCI Operations. 
   12. SDP Operations. 
   13. SDDB Operations. 
   14. RFCOMM Operations. 
   15. MCAP Operations. 
   16. BNEP Operations. 
   17. AVDTP Operations. 
   18. AVCTP Operations. 
   19. OBEX Client Operations. 
   20. OBEX Server Operations. 
   21. L2CAP Operations. 
 
   25. Write Storage event mask. 
 
   30. Security Manager. 
   31. LE SMP. 
   32. LE L2CAP. 
   33. ATT. 
   34. L2CAP ECBFC. 
 
   40. SPP Operations. 
   41. HFP Unit Operations. 
   42. HFP AG Operations. 
   45. DUNP DT Operations. 
   46. DUNP GW Operations. 
   47. SAP Client Operations. 
   48. SAP Server Operations. 
 
   50. OPP Client Operations. 
   51. OPP Server Operations. 
   52. FTP Client Operations. 
   53. FTP Server Operations. 
   54. MAP Client Operations. 
   55. MAP Server Operations. 
   56. PBAP Client Operations. 
   57. PBAP Server Operations. 
   58. CTN Client Operations. 
   59. CTN Server Operations. 
   60. BIP Initiator Operations. 
   61. BIP Responder Operations. 
   62. SYNCP Client Operations. 
   63. SYNCP Server Operations. 
 
   65. A2DP Operations. 
   66. AVRCP Operations. 
   67. HDP Operations. 
   68. PAN Operations. 
 
   70. HID Device Operations. 
   71. HID Host Operations. 
 
   75. DID Client Operations. 
   76. DID Server Operations. 
 
   80. GATT Client Operations. 
   81. GATT Server Operations. 
 
   90. BPP Sender Operations. 
   91. BPP Printer Operations. 
 
  100. Simulate VU. 
 
  200. GA Legacy Options. 
 
  201. GA CCP Client Operations. 
  202. GA CCP Server Operations. 
 
  203. GA MCP Client Operations. 
  204. GA MCP Server Operations. 
 
  205. GA TMAP Operations. 
  206. GA HAP Operations. 
  207. GA BASS Operations.
 
  210. GA Setup. 
 
  220. GA Profile Options.
 
  250. Wake on BLE vendor command. 
  251. H2C sleep vendor command. 
  252. H2C sleep. 
  253. H2C wakeup. 
  254. Host sleep. 
 
  280. Disable Logging.
  281. Enable Logging.
 
  300. BEACON Manager 
 
 Your Option ?2
choice is 2
Initializing EtherMind ...
EtherMind Stack Successfully Initialized!
 
   0.  Exit. 
   1.  Refresh this Menu. 
 
   2.  EtherMind Init. 
   3.  Bluetooth ON. 
   4.  Bluetooth OFF. 
 
   5.  Set PIN Code. 
   6.  Set Device Role. 
 
   7.  Get Free ACL Tx Queue Buffer. 
   8.  Set LWM/HWM for ACL Tx Queue. 
 
   9.  Configure Service GAP Procedures 
   10. Set Snoop logging. 
 
   11. HCI Operations. 
   12. SDP Operations. 
   13. SDDB Operations. 
   14. RFCOMM Operations. 
   15. MCAP Operations. 
   16. BNEP Operations. 
   17. AVDTP Operations. 
   18. AVCTP Operations. 
   19. OBEX Client Operations. 
   20. OBEX Server Operations. 
   21. L2CAP Operations. 
 
   25. Write Storage event mask. 
 
   30. Security Manager. 
   31. LE SMP. 
   32. LE L2CAP. 
   33. ATT. 
   34. L2CAP ECBFC. 
 
   40. SPP Operations. 
   41. HFP Unit Operations. 
   42. HFP AG Operations. 
   45. DUNP DT Operations. 
   46. DUNP GW Operations. 
   47. SAP Client Operations. 
   48. SAP Server Operations. 
 
   50. OPP Client Operations. 
   51. OPP Server Operations. 
   52. FTP Client Operations. 
   53. FTP Server Operations. 
   54. MAP Client Operations. 
   55. MAP Server Operations. 
   56. PBAP Client Operations. 
   57. PBAP Server Operations. 
   58. CTN Client Operations. 
   59. CTN Server Operations. 
   60. BIP Initiator Operations. 
   61. BIP Responder Operations. 
   62. SYNCP Client Operations. 
   63. SYNCP Server Operations. 
 
   65. A2DP Operations. 
   66. AVRCP Operations. 
   67. HDP Operations. 
   68. PAN Operations. 
 
   70. HID Device Operations. 
   71. HID Host Operations. 
 
   75. DID Client Operations. 
   76. DID Server Operations. 
 
   80. GATT Client Operations. 
   81. GATT Server Operations. 
 
   90. BPP Sender Operations. 
   91. BPP Printer Operations. 
 
  100. Simulate VU. 
 
  200. GA Legacy Options. 
 
  201. GA CCP Client Operations. 
  202. GA CCP Server Operations. 
 
  203. GA MCP Client Operations. 
  204. GA MCP Server Operations. 
 
  205. GA TMAP Operations. 
  206. GA HAP Operations. 
  207. GA BASS Operations.
 
  210. GA Setup. 
 
  220. GA Profile Options.
 
  250. Wake on BLE vendor command. 
  251. H2C sleep vendor command. 
  252. H2C sleep. 
  253. H2C wakeup. 
  254. Host sleep. 
 
  280. Disable Logging.
  281. Enable Logging.
 
  300. BEACON Manager 
 
 Your Option ?3
choice is 3
Performing Bluetooth ON ...

Default GAP Role Set to PERIPHERAL
Initialize all required GA modules and then register the database.

 
   0.  Exit. 
   1.  Refresh this Menu. 
 
   2.  EtherMind Init. 
   3.  Bluetooth ON. 
   4.  Bluetooth OFF. 
 
   5.  Set PIN Code. 
   6.  Set Device Role. 
 
   7.  Get Free ACL Tx Queue Buffer. 
   8.  Set LWM/HWM for ACL Tx Queue. 
 
   9.  Configure Service GAP Procedures 
   10. Set Snoop logging. 
 
   11. HCI Operations. 
   12. SDP Operations. 
   13. SDDB Operations. 
   14. RFCOMM Operations. 
   15. MCAP Operations. 
   16. BNEP OperaReceived HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x0C56 (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x1005 (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x0C13 (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x0C18 (UNKNOWN)

>	Bluetooth ON Initialization Completed.
>	Bluetooth Address: 00:00:00:00:00:00

>	Stack Version - 020.006.000.
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0xFC0F (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x080F (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x0C1A (UNKNOWN)
Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x01
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x0C6D (UNKNOWN)
tions. 
   17. AVDTP Operations. 
   18. AVCTP Operations. 
   19. OBEX Client Operations. 
   20. OBEX Server Operations. 
   21. L2CAP Operations. 
 
   25. Write Storage event mask. 
 
   30. Security Manager. 
   31. LE SMP. 
   32. LE L2CAP. 
   33. ATT. 
   34. L2CAP ECBFC. 
 
   40. SPP Operations. 
   41. HFP Unit Operations. 
   42. HFP AG Operations. 
   45. DUNP DT Operations. 
   46. DUNP GW Operations. 
   47. SAP Client Operations. 
   48. SAP Server Operations. 
 
   50. OPP Client Operations. 
   51. OPP Server Operations. 
   52. FTP Client Operations. 
   53. FTP Server Operations. 
   54. MAP Client Operations. 
   55. MAP Server Operations. 
   56. PBAP Client Operations. 
   57. PBAP Server Operations. 
   58. CTN Client Operations. 
   59. CTN Server Operations. 
   60. BIP Initiator Operations. 
   61. BIP Responder Operations. 
   62. SYNCP Client Operations. 
   63. SYNCP Server Operations. 
 
   65. A2DP Operations. 
   66. AVRCP Operations. 
   67. HDP Operations. 
   68. PAN Operations. 
 
   70. HID Device Operations. 
   71. HID Host Operations. 
 
   75. DID Client Operations. 
   76. DID Server Operations. 
 
   80. GATT Client Operations. 
   81. GATT Server Operations. 
 
   90. BPP Sender Operations. 
   91. BPP Printer Operations. 
 
  100. Simulate VU. 
 
  200. GA Legacy Options. 
 
  201. GA CCP Client Operations. 
  202. GA CCP Server Operations. 
 
  203. GA MCP Client Operations. 
  204. GA MCP Server Operations. 
 
  205. GA TMAP Operations. 
  206. GA HAP Operations. 
  207. GA BASS Operations.
 
  210. GA Setup. 
 
  220. GA Profile Options.
 
  250. Wake on BLE vendor command. 
  251. H2C sleep vendor command. 
  252. H2C sleep. 
  253. H2C wakeup. 
  254. Host sleep. 
 
  280. Disable Logging.
  281. Enable Logging.
 
  300. BEACON Manager 
 
 Your Option ?220
choice is 220

 
    0. Exit. 
    1. Refresh this Menu. 

    2. GA Init. 
    3. GA Setup. 
    4. GA Shutdown. 

    5. GA Register - PACS. 
    6. Notify PACS - Audio Capability. 
    7. Notify PACS - Available Audio Contexts. 
    8. Notify PACS - Supported Audio Contexts. 
    9. Notify PACS - Audio Locations. 

   10. Setup PACS - Audio Role Discovery. 
   11. Release PACS. 
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts 

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral] 
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral] 
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central] 
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]  

   25. LE Create Connection. [CAP - Initiator GAP - Central] 
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]  
   27. LE Legacy Authenticate and Encrypt. 
   28. LE LESC Authenticate and Encrypt. 
   29. Disconnection. 

   30. Display all Connected Devices. 
   31. Set Connected Device Index. 
   32. Remove Device from Device Manager. 

   35. Configure Sink Audio Dump. 
   36. Configure Source Audio Dump. 

   40. Unicast Operations. 
   45. Broadcast Operations. 

   50. VCP Volume Controller Operations. 
   51. VCP Volume Renderer Operations. 

   55. MICP Microphone Controller Operations. 
   56. MICP Microphone Device Operations. 

   60. CSIP Set Coordinator and Set Member Operations. 

   65. MCP Media Control Client. 
   66. MCP Media Control Server. 

   70. CCP Call Control Client. 
   71. CCP Call Control Server. 

   75. GA TMAP Operations. 
   76. GA HAP Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

   90. Configure Notification 

Your Option ?: 2
Registering HCI Callback Function at Location 0
Number of Registered HCI Callback Functions = 1
HCI Callback Registered
Configuring GA
Initializing GA
Registering HCI Callback Function at Location 1
Number of Registered HCI Callback Functions = 2
Retval - GA_SUCCESS (0x0000)
Default settings done for device manager
GA_vc_register. for VOCS 
VCP_VOCS Handle: 0x00
Retval - 0x0000
GA_vc_register..for AICS 
VCP_AICS Handle: 0x00
Retval - 0x0000
GA_mc_register_aics 
AICS Handle: 0x00
Retval - GA_SUCCESS (0x0000)

 
    0. Exit. 
    1. Refresh this Menu. 

    2. GA Init. 
    3. GA Setup. 
    4. GA Shutdown. 

    5. GA Register - PACS. 
    6. Notify PACS - Audio Capability. 
    7. Notify PACS - Available Audio Contexts. 
    8. Notify PACS - Supported Audio Contexts. 
    9. Notify PACS - Audio Locations. 

   10. Setup PACS - Audio Role Discovery. 
   11. Release PACS. 
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts 

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral] 
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral] 
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central] 
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]  

   25. LE Create Connection. [CAP - Initiator GAP - Central] 
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]  
   27. LE Legacy Authenticate and Encrypt. 
   28. LE LESC Authenticate and Encrypt. 
   29. Disconnection. 

   30. Display all Connected Devices. 
   31. Set Connected Device Index. 
   32. Remove Device from Device Manager. 

   35. Configure Sink Audio Dump. 
   36. Configure Source Audio Dump. 

   40. Unicast Operations. 
   45. Broadcast Operations. 

   50. VCP Volume Controller Operations. 
   51. VCP Volume Renderer Operations. 

   55. MICP Microphone Controller Operations. 
   56. MICP Microphone Device Operations. 

   60. CSIP Set Coordinator and Set Member Operations. 

   65. MCP Media Control Client. 
   66. MCP Media Control Server. 

   70. CCP Call Control Client. 
   71. CCP Call Control Server. 

   75. GA TMAP Operations. 
   76. GA HAP Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

   90. Configure Notification 

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)

 
    0. Exit. 
    1. Refresh this Menu. 

    2. GA Init. 
    3. GA Setup. 
    4. GA Shutdown. 

    5. GA Register - PACS. 
    6. Notify PACS - Audio Capability. 
    7. Notify PACS - Available Audio Contexts. 
    8. Notify PACS - Supported Audio Contexts. 
    9. Notify PACS - Audio Locations. 

   10. Setup PACS - Audio Role Discovery. 
   11. Release PACS. 
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts 

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral] 
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral] 
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central] 
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]  

   25. LE Create Connection. [CAP - Initiator GAP - Central] 
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]  
   27. LE Legacy Authenticate and Encrypt. 
   28. LE LESC Authenticate and Encrypt. 
   29. Disconnection. 

   30. Display all Connected Devices. 
   31. Set Connected Device Index. 
   32. Remove Device from Device Manager. 

   35. Configure Sink Audio Dump. 
   36. Configure Source Audio Dump. 

   40. Unicast Operations. 
   45. Broadcast Operations. 

   50. VCP Volume Controller Operations. 
   51. VCP Volume Renderer Operations. 

   55. MICP Microphone Controller Operations. 
   56. MICP Microphone Device Operations. 

   60. CSIP Set Coordinator and Set Member Operations. 

   65. MCP Media Control Client. 
   66. MCP Media Control Server. 

   70. CCP Call Control Client. 
   71. CCP Call Control Server. 

   75. GA TMAP Operations. 
   76. GA HAP Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

   90. Configure Notification 

Your Option ?: 75
    0. Exit 
    1. Refresh this Menu 

    2. GA TMAP Initialize 
    3. GA TMAP Register 
    4. GA TMAP Shutdown 

   10. Discover TMAS Role 
   11. Release TMAS 

   20. Unicast Client. 
   21. VCP Volume Controller Operations. 
   22. CCP Call Control Server. 

   30. Unicast Server. 
   31. VCP Volume Renderer Operations. 

   40. Unicast Client - BAP Audio Source. 
   41. VCP Volume Controller Operations. 
   42. MCP Media Control Server. 

   50. Unicast Server - BAP Audio Sink. 
   51. VCP Volume Renderer Operations. 

   60. Broadcast Source. 

   70. Broadcast Sink. 
   71. VCP Volume Renderer Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

Your Option? 2
GA_tmap_init...
Retval - GA_SUCCESS (0x0000)

    0. Exit 
    1. Refresh this Menu 

    2. GA TMAP Initialize 
    3. GA TMAP Register 
    4. GA TMAP Shutdown 

   10. Discover TMAS Role 
   11. Release TMAS 

   20. Unicast Client. 
   21. VCP Volume Controller Operations. 
   22. CCP Call Control Server. 

   30. Unicast Server. 
   31. VCP Volume Renderer Operations. 

   40. Unicast Client - BAP Audio Source. 
   41. VCP Volume Controller Operations. 
   42. MCP Media Control Server. 

   50. Unicast Server - BAP Audio Sink. 
   51. VCP Volume Renderer Operations. 

   60. Broadcast Source. 

   70. Broadcast Sink. 
   71. VCP Volume Renderer Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
0
 - Call Gateway(CG): 0x0001
 - Call Terminal(CT): 0x0002
 - Unicast Media Sender (UMS): 0x0004
 - Unicast Media Receiver (UMR): 0x0008
 - Broadcast Media Sender (BMS): 0x0010
 - Broadcast Media Receiver (BMR): 0x0020
Note: If this device supports more than one role, Compute BitMask OR and input
E.g., Call Gateway(CG):0x0001 and Call Terminal(CT):0x0002, Input 0x0003
20
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)

    0. Exit 
    1. Refresh this Menu 

    2. GA TMAP Initialize 
    3. GA TMAP Register 
    4. GA TMAP Shutdown 

   10. Discover TMAS Role 
   11. Release TMAS 

   20. Unicast Client. 
   21. VCP Volume Controller Operations. 
   22. CCP Call Control Server. 

   30. Unicast Server. 
   31. VCP Volume Renderer Operations. 

   40. Unicast Client - BAP Audio Source. 
   41. VCP Volume Controller Operations. 
   42. MCP Media Control Server. 

   50. Unicast Server - BAP Audio Sink. 
   51. VCP Volume Renderer Operations. 

   60. Broadcast Source. 

   70. Broadcast Sink. 
   71. VCP Volume Renderer Operations. 

   80. Get Context Info. 
   81. Set Context Info. 

   87. Free Saved Context Info. 

Your Option? 70
 
    0. Exit. 
    1. Refresh this Menu. 

    2. SD Init. 
    3. SD Shutdown. 
    4. Add Broadcast Receive State Char Instance. 
    5. Start SD Advertising. 
    6. Stop SD Advertising. 

   10. Send Broadcast Receive State Notification. 

   15. Request Sync Info (PAST - Available). 
   16. Scan and Associate to Broadcast Source (PAST - Not Available). 
   17. Dissociate from Broadcast Source and Stop Scanning. 
   18. Enable Stream. 
   19. Disable Stream. 

   25. Scan Announcement. 
   26. Scan End. 
   27. Associate Source. 
   28. Dissociate Source. 
   29. Enable Stream. 
   30. Disable Stream. 

Your Option ?: 25
Setup Broadcast Sink...
Retval - GA_SUCCESS (0x0000)

************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SCAN_ANNOUNCEMENT_CNF (0xC5)

-----------------------------------------------------------------
Data Length: 0x00
Data: NULL
************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SOURCE_ANNOUNCEMENT_IND (0xCD)

-----------------------------------------------------------------
Len: 0x02,  Event Type: 0x0000
Len: 0x01,  Address Type: 0x00
Len: 0x06,  Address: D4:BD:27:E8:07:C0
Len: 0x01,  Primary Phy: 0x01
Len: 0x01,  Secondary Phy: 0x01
Len: 0x01,  SID: 0x01
Len: 0x01,  Tx Power: 127
Len: 0x01,  RSSI: -48
Len: 0x02,  PA Interval: 0x0010
Len: 0x01,  Direct Address Type: 0x00
Len: 0x06,  Direct Address: 00:00:00:00:00:00
Data Length: 0x07
Data:
Len: 0x03,  Broadcast_ID: 0xDEADBE
		- AD Length: 0x0006
		- AD Type: 0x0016
		- AD Value: Service UUID: Broadcast Audio Announcement Service (0x1852)


 
    0. Exit. 
    1. Refresh this Menu. 

    2. SD Init. 
    3. SD Shutdown. 
    4. Add Broadcast Receive State Char Instance. 
    5. Start SD Advertising. 
    6. Stop SD Advertising. 

   10. Send Broadcast Receive State Notification. 

   15. Request Sync Info (PAST - Available). 
   16. Scan and Associate to Broadcast Source (PAST - Not Available). 
   17. Dissociate from Broadcast Source and Stop Scanning. 
   18. Enable Stream. 
   19. Disable Stream. 

   25. Scan Announcement. 
   26. Scan End. 
   27. Associate Source. 
   28. Dissociate Source. 
   29. Enable Stream. 
   30. Disable Stream. 

Your Option ?: 27
Enter SID: 1
Enter endpoint address: D4 BD 27 E8 07 C0
BD_ADDR Entered D4BD27E807C0
Enter endpoint type: 0
Associate Broadcast Sink...
Retval - Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x00
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x2044 (HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_OPCODE)
GA_SUCCESS (0x0000)

 
    0. Exit. 
    1. Refresh this Menu. 

    2. SD Init. 
    3. SD Shutdown. 
    4. Add Broadcast Receive State CSubevent : HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_SUBEVENT.
status: 0x00
Sync_Handle: 0x0000
Periodic_Advertising_ID: 0x01
Advertiser_Address_Type: 0x00
Advertiser_Address: 
Advertiser_PHY: 0x01
Periodic_Advertising_Interval: 0x0010
Advertiser_Clock_Accuracy: 0x07
************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_ASSOCIATE_CNF (0xC7)

-----------------------------------------------------------------
Data Length: 0x0F
Data:
Len: 0x01,  Error: 0x00
Len: 0x02,  Sync Handle: 0x0000
Len: 0x01,  SID: 0x01
Len: 0x01,  Advertiser Address Type: 0x00
Len: 0x06,  Advertiser Address: D4:BD:27:E8:07:C0
Len: 0x01,  Adv Phy: 0x01
Len: 0x02,  PA Interval: 0x0010
Len: 0x01,  Adv Clock Accuracy: 0x07
************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SOURCE_CONFIG_IND (0xCE)

-----------------------------------------------------------------
Len: 0x02,  Sync Handle: 0x0000
Len: 0x01,  Tx Power: 0
Len: 0x01,  RSSI: -46
Len: 0x01,  CTE Type: 0xFF
Len: 0x01,  Data Status: 0x00
Data Length: 0x34
Data:
Len: 0x03,  Presentation_Delay: 0x9C40
Len: 0x01,  Num_Subgroups: 0x01
Subgroups[1]:
Len: 0x01,  Num_BIS: 0x01
Len: 0x05,  Codec ID:
		- Len: 0x01,  Coding Format: LC3 (0x06),
		- Len: 0x02,  Company ID: (0x0000)
		- Len: 0x02,  Vendor Specific Codec ID: (0x0000)
Len: 0x01,  Codec_Specific_Configuration_Length: 0x13
Len: 0x13,  Codec_Specific_Configuration:
		- Len: 0x01,  Length: 0x02
		- Len: 0x01,  Type: 0x01
		- Len: 0x01,  Value: Sampling Frequencies: 48000 Hz (0x0008)

		- Len: 0x01,  Length: 0x02
		- Len: 0x01,  Type: 0x02
		- Len: 0x01,  Value: Frame Durations: 10 ms (0x01)

		- Len: 0x01,  Length: 0x05
		- Len: 0x01,  Type: 0x03
		- Len: 0x04,  Value: Audio Channel Allocation: 
		- Front Left (0x1)

		- Len: 0x01,  Length: 0x03
		- Len: 0x01,  Type: 0x04
		- Len: 0x02,  Value: Octets per Codec Frame: 0x0064

		- Len: 0x01,  Length: 0x02
		- Len: 0x01,  Type: 0x05
		- Len: 0x01,  Value: Codec Frame Blocks Per SDU: 0x01

Len: 0x01,  Metadata_Length: 0x09
Len: 0x09,  Metadata:
		- Len: 0x01,  Length: 0x03
		- Len: 0x01,  Type: 0x02
		- Len: 0x02,  Value: Streaming Audio Contexts: 	- Media (0x0004)

		- Len: 0x01,  Length: 0x04
		- Len: 0x01,  Type: 0x04
		- Len: 0x03,  Value: Language: eng

Len: 0x01,  BIS_index: 0x01
Len: 0x01,  Codec_Specific_Configuration_Length: 0x06
Len: 0x06,  Codec_Specific_Configuration:
		- Len: 0x01,  Length: 0x05
		- Len: 0x01,  Type: 0x03
		- Len: 0x04,  Value: Audio Channel Allocation: 
		- Front Left (0x1)

har Instance. 
    5. Start SD Advertising. 
    6. Stop SD Advertising. 

   10. Send Broadcast Receive State Notification. 

   15. Request Sync Info (PAST - Available). 
   16. Scan and Associate to Broadcast Source (PAST - Not Available). 
   17. Dissociate from Broadcast Source and Stop Scanning. 
   18. Enable Stream. 
   19. Disable Stream. 

   25. Scan Announcement. 
   26. Scan End. 
   27. Associate Source. 
   28. Dissociate Source. 
   29. Enable Stream. 
   30. Disable Stream. 

Your Option ?: ************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SOURCE_STREAM_IND (0xCF)

-----------------------------------------------------------------
Data Length: 0x13
Data:
Len: 0x02,  Sync Handle: 0x0000
Len: 0x01,  Num BIS: 0x01
Len: 0x01,  NSE: 0x03
Len: 0x02,  ISO Interval: 0x0008
Len: 0x01,  BN: 0x01
Len: 0x01,  PTO: 0x00
Len: 0x01,  IRC: 0x03
Len: 0x02,  Max PDU: 0x0064
Len: 0x04,  SDU Interval: 0x00002710
Len: 0x02,  Max SDU: 0x0064
Len: 0x01,  Phy: 0x02
Len: 0x01,  Framing: 0x00
Len: 0x01,  Encryption: 0x00
************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_SOURCE_ANNOUNCEMENT_IND (0xCD)

-----------------------------------------------------------------
Len: 0x02,  Event Type: 0x0000
Len: 0x01,  Address Type: 0x00
Len: 0x06,  Address: D4:BD:27:E8:07:C0
Len: 0x01,  Primary Phy: 0x01
Len: 0x01,  Secondary Phy: 0x01
Len: 0x01,  SID: 0x01
Len: 0x01,  Tx Power: 127
Len: 0x01,  RSSI: -47
Len: 0x02,  PA Interval: 0x0010
Len: 0x01,  Direct Address Type: 0x00
Len: 0x06,  Direct Address: 00:00:00:00:00:00
Data Length: 0x07
Data:
Len: 0x03,  Broadcast_ID: 0xDEADBE
		- AD Length: 0x0006
		- AD Type: 0x0016
		- AD Value: Service UUID: Broadcast Audio Announcement Service (0x1852)


1
 
    0. Exit. 
    1. Refresh this Menu. 

    2. SD Init. 
    3. SD Shutdown. 
    4. Add Broadcast Receive State Char Instance. 
    5. Start SD Advertising. 
    6. Stop SD Advertising. 

   10. Send Broadcast Receive State Notification. 

   15. Request Sync Info (PAST - Available). 
   16. Scan and Associate to Broadcast Source (PAST - Not Available). 
   17. Dissociate from Broadcast Source and Stop Scanning. 
   18. Enable Stream. 
   19. Disable Stream. 

   25. Scan Announcement. 
   26. Scan End. 
   27. Associate Source. 
   28. Dissociate Source. 
   29. Enable Stream. 
   30. Disable Stream. 

Your Option ?: 29
Enter Num_BIS: 1
Enter BIS_Index [IN HEX]: 1
Enable Broadcast Sink...
Retval - GA_SUCCESS (0x0000)

Received HCI_COMMAND_STATUS_EVENT In LE.
	LE Command Status: 0x00
	LE Num Command Packets: 1 (0x01)
	LE Command Opcode: 0x206B (HCI_LE_BIG_CREATE_SYNC_OPCODE)
 
    0. Exit. 
    1. Refresh this Menu. 

    2. SD Init. 
    3. SD Shutdown. 
    4. Add Broadcast Receive State CSubevent : HCI_LE_BIG_SYNC_ESTABLISHED_SUBEVENT.
Status: 0x00
BIG_Handle: 0x00
Transport_Latency_BIG: 0x00000000
Num_BIS: 0x03
Connection Handles of BISes in the BIG
************************** CALLBACK: GA BC CAP Server **************************
[Profile]     : CAP 
[SECTION]     : BCAS SINK EVENTS
[SUB-SECTION] : BCAS SINK-SETUP
[TYPE]        : SETUP RESPONSE
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_BC_ENABLE_CNF (0xCA)

-----------------------------------------------------------------
Data Length: 0x10
Data:
Len: 0x01,  Error Code: 0x00
Len: 0x01,  BIG Handle: 0x00
Len: 0x04,  Transport Latency BIG: 0x00000000
Len: 0x01,  NSE: 0x03
Len: 0x01,  BN: 0x01
Len: 0x01,  PTO: 0x00
Len: 0x01,  IRC: 0x03
Len: 0x02,  Max PDU: 0x0064
Len: 0x02,  ISO Interval: 0x0008
Len: 0x01,  Number of BISes: 0x01
BIS[1]:
	Len: 0x02,  BIS Conn Handle: 0x0006
[APPL][GA][BC_SNK]: ISO Data Path Setup Status: In-progress
HCI_COMMAND_COMPLETE_EVENT
[APPL][GA][BC_SNK]: ISO Data Path Setup Status: Complete
[APPL][GA][BC_SNK]: LC3 Decoder Setup Status: Created
Init Audio SAI and CODEC
***change setting**********
switch to 48khz[APPL][GA][BC_SNK]: Audio PL Playback Setup Status: Success
[APPL][GA][BC_SNK]: Audio PL Playback Start Process Status: Success
har Instance. 
    5. Start SD Advertising. 
    6. Stop SD Advertising. 

   10. Send Broadcast Receive State Notification. 

   15. Request Sync Info (PAST - Available). 
   16. Scan and Associate to Broadcast Source (PAST - Not Available). 
   17. Dissociate from Broadcast Source and Stop Scanning. 
   18. Enable Stream. 
   19. Disable Stream. 

   25. Scan Announcement. 
   26. Scan End. 
   27. Associate Source. 
   28. Dissociate Source. 
   29. Enable Stream. 
   30. Disable Stream. 

Your Option ?: 1


################################
TMAP BMR
################################





 8. Now, Initialize and Start any required profile from the corresponding menu option. At this point, the stack is ready to accept incoming
    connections from any peer device (PTS/Mobile/EtherMind application/PC Stack) for the started profile.

   8.0 Switch the peer device when have multiple connections

      When central device connected to multiple devices, the default peer device will set to newest. 
      For example, a device's HTS and PXM may implement at the same time.
      First, PXM connect to PXR device, then a Phone connect to HTS. 
      Now the default peer device is Phone, the PXM functions like write/read will not work because current ATT Handle is for Phone.
      In order to change peer device to make PXM work again, the following process must be used:

      1). Input 80 in "MAIN MENU" into "GATT CLIENT MENU". 
      2). Input 2  in "GATT CLIENT MENU" into "Register GATT Handle" options, there will be a devices list.

            ***************************************
            List of Current Connected devices:
            ***************************************

            1. Peer with - 
                 ->         APPL_HANDLE: 1
                 ->   Connection Handle: 0x0080
                 ->                ADDR: 4E 7F 38 37 60 00, TYPE: 00
                 ->         ATT Handle : DEV_ID - 0x0C ATT_ID - 0x01
            2. Peer with - 
                 ->         APPL_HANDLE: 0
                 ->   Connection Handle: 0x0081
                 ->                ADDR: 90 AE 72 68 BD 62, TYPE: 01
                 ->         ATT Handle : DEV_ID - 0x0B ATT_ID - 0x00
            Enter Device ID: 0C
            Enter ATT ID: 01

      3). Distinguish peer device by MAC address, then input target peer device's "Device ID" and "ATT ID".
          Now the peer device have changed to target.

   8.1 Running Sample SPP Application
      a) Running Serial Port Profile - Server
         1). Setup and prepare the application.
             - Enter option 2 (EtherMind Init) in Main menu.
             - Enter option 3 (Bluetooth ON) in Main menu.
         2). Setup the SPP Unit profile
             - Enter option 40 (SPP Operations) in Main menu.
             - Enter option 3 (SPP Init) in SPP menu.
             - Enter option 4 (Start SPP) in SPP menu.
             - Enter option 5 (Start SPP Vendor Service) in SPP menu (Optional)

             Notice: Now, SPP server is started and ready to accept incoming SPP connections from clients.

      b) Running Serial Port Profile - Client
         1). Setup and prepare the application.
             - Enter option 2 (EtherMind Init) in Main menu.
             - Enter option 3 (Bluetooth ON) in Main menu.
         2). Setup the SPP Unit profile
             - Enter option 40 (SPP Operations) in Main menu.
             - Enter option 3 (SPP Init) in SPP menu.
             - Enter option 4 (Start SPP) in SPP menu.
             - Enter option 2 (Set BD_ADDR of Peer SPP) in SPP menu.
               > Enter the peer BD Address (in LSB to MSB order separated by space. Eg: 0x001BDCE0384C is the peer address,enter as "4C 38 E0 DC 1B 00")
             - Enter option 10 (Create ACL Connection) in SPP menu
             - Enter option 11 (Get SPP SDP Record) in SPP menu.
             - Enter option 20 (Connect to SPP) in SPP menu.

             Notice: Now, SPP connection is created.

      c) After SPP connection is created, the following steps can be performed on server or client
         1). Send data to peer device
             - Enter option 30 (Send data over SPP) in SPP menu.
               > Enter the Sample Strings index
         2). Save data received over the connected SPP channel in a file
             - Connect U-disk to RT1060 board's J9 with OTG
             - Enter option 50 (Receive file) in SPP menu
               > Enter file name
               > Now received data will be saved in created file. 
             - Enter option 51 (Stop Receiving file) in SPP menu to stop receive data
         3). Disconnect from peer SPP
             - Enter option 21 (Disconnect from Peer SPP) in SPP menu.
   8.2 Running Sample HFP Application
      a) Running Hands Free Profile Audio Gateway
         1). Refer to main menu 1-7 steps to setup and prepare the application.
         2). Setup the HFP AG profile
             - Enter option 42 (HFP AG Operations) in Main menu.
             - Enter option 2 (Initialize) in HFP AG menu. Note for successful profile initialization
             - Enter option 3 (Start) in HFP AG menu
               > Enter Local supported features -> 16383 (All feature selection)
             - Note for successful profile startup.
         3). In order to connect to a peer HFP unit
             - Enter option 10 in HFP AG menu, register the peer HFP Unit BD Address.
             - Enter option 12 to connect to the peer Unit.
             - After Successful HCI_CONNECTION_COMPLETE_EVENT with Success (0x00) status, enter option 11 to  get the Unit server channel.
             - Enter option 4 to initiate the HFP SLC procedure. The following kind of prints should be printed on console.
         4). Notify AG indicators to Unit
             - Enter option 60 to configure and notify Inband ringtone setting
             - Enter option 62 to configure and notify of the HF indicators
         5). Gain Control
             - Enter option 63 to configure and notify of the Speaker gain
             - Enter option 64 to configure and notify of the Microphone gain
         6). Voice Recognition
             - Enter option 43 to enable voice recognition notification to HFP Unit
             - Enter option 44 to disable voice recognition notification to HFP Unit
             - Enter option 43 to notify enhanced voice recognition indications to HFP Unit
         7). Call Handling
             - Use the menu option 100 to manage the call and network settings with the simple network simulator
         8). Audio Transfer
             - Enter option 14 to transfer the voice audio when in call to the HFP Unit.
             - Enter option 15 to transfer the voice audio when in call to the HFP AG.    
      b) Running Hands Free Profile Unit
         1). Refer to main menu 1-7 steps to setup and prepare the application.
         2). Setup the HFP Unit profile
             - Enter option 41 (HFP unit Operations) in Main menu.
             - Enter option 4 (Initialize) in HFP Unit menu. Note for successful profile initialization
             - Enter option 5 (Start) in HFP Unit menu
               > Enter Local supported features; 1023
               > Enter available codecs; 1 or 1,2
               > Enter available HF indicators; 1, 2 or 1,2
             - Note for successful profile startup.
         3). Discover the device from a peer mobile phone and connect to it. Make sure should be displayed on the terminal.
         4). Enable/Disable HFP features
             - Enter options 50/51 to enable/disable Caller Line Identification Notification (CLIP)
             - Enter options 52/53 to enable/disable Call Waiting Notification (CCWA)
             - Enter options 54/55 to enable/disable Voice Recognition (BVRA)
             - Enter options 56 to disable ECNR in AG
         5). Gain Control
             - Enter option 60 to set the speaker gain level
             - Enter option 61 to set the microphone gain level
             - Enter option 62 to get the current gain level configured. 
               - Note that upon establishing a new service level connection, this option might return an incorrect value.
                 The application should set the gain using the above options after every new connection before trying to
                 fetch the current gain level.
         6). Incoming Call
             - Enter option 40 to accept an incoming call
             - Enter option 41 to reject an incoming call
         7). Outgoing Call
             - Enter option 30 to dial to a given number
             - Enter option 31 to dial to a number in a given memory location in AG
             - Enter option 32 to redial the last call on the AG
         8). Active Call
             - Enter option 42 to hangup an active call
         9). Three-Way Call 
             - Enter option 45->0 to release all held calls or Reject waiting call
             - Enter option 45->1 to release active call and accept held/waiting call
             - Enter option 45->2 to hold active call and accept held/waiting call
             - Enter option 45->3 to conference the calls
             - Enter option 45->4 to join calls and disconnect self
         10). Codec Negotiation & Trigger codec connection
             - Enter option 75->6 to update available codecs information to AG
             - Enter option 75->8 to trigger a codec connection with the AG   
             
         Notice:
            1. Headset is connected to J12 of RT1060 board.
            2. In order to initiate connection from local side to a peer HFP AG mobile:
             - Enter option 2 (Register BD Addr) the peer mobile BD Address in LSB-MSB format
             - Enter option 7 (Create ACL) to setup ACL link with registered peer device, and wait for Successful HCI Connection event as described .
             - Enter option 8 (Get AG SDP) to read the server channel of the AG service in the peer device.
             - Enter option 9 (Establish SLC) to make the HFP service level connection with the peer device. On successful connection, the following prints will be displayed on the terminal

             > Event          : HFP_UNIT_CONNECT_CNF
             > Instance       : 0x00
             > Event result   : 0x0000
             > BD_ADDR of peer F1:CE:D4:F9:A2:64
    8.3 Running Sample A2DP Application
       a) Running Advanced Audio Distribution Profile - Source
          1). Setup and prepare the application.
              - Enter option 2 (EtherMind Init) in Main menu.
              - Enter option 3 (Bluetooth ON) in Main menu.
          2). Setup the A2DP profile
              - Enter option 65 (A2DP Operations) in Main menu.
              - Enter option 2 (A2DP Initialize) in A2DP menu
              - Enter option 4 (A2DP Register Codec) in A2DP menu
                > Enter Source Endpoint; 0
                > Enter codec type SBC; 0
          3). Make the A2DP Sink in discoverable mode, search and create and ACL connection
              - Enter option 11 (HCI Operations) in Main menu
              - Enter option 10 (Create Connection (ACL));
                > Enter the peer BD Address (in LSB to MSB order separated by space. Eg: 0x001BDCE0384C is the peer address,enter as "4C 38 E0 DC 1B 00")
                > Enter Clock Offset as 0
                > Enter Role Switch as 1
          4). Create and AVDTP transport connection as below
              - Enter option 10 (AVDTP Connect) in A2DP menu
                > Enter the ACL connection index, 0
              - On successful AVDTP transport connection, the following prints should be on the terminal.

                Received AVDTP_CONNECT_CNF
                        AVDTP Handle:
                            ADDR: E2:E2:A7:B8:70:E4
                        Result = 0x0000
                      
          5). Discover the Endpoints in the peer device
              - Enter option 11 (AVDTP Discover) in A2DP menu
                > Enter the ACL connection index, 0
              - On successful discovery completion, the following prints should be on the terminal.

                Received AVDTP_DISCOVER_CNF
                        AVDTP Handle:
                            ADDR: E2:E2:A7:B8:70:E4
                            Remote SEID = 0x00
                        Result = 0x0000
                        Event Data = 2022A668, Data Length = 2
                Response Buf [0] = 0x04, [1] = 0x08
                        Remote SEP Information [0]:
                            In Use     = No
                            SEP Type   = 0x01 -> Sink
                            ACP SEID   = 0x01
                            Media Type = 0x00 -> Audio
                           
          6a). Get the capabilities of the required endpoint ID as displayed in the Discover response
               - Enter option 12 (AVDTP Get Capabilities) in A2DP menu
                 > Enter the remote Stream Endpoint Identifier (SEID), 1 (ACP SEID) in this case is the Sink endpoint as in response above
               - On successful procedure completion, the following prints should be on the terminal. This displays the codec type and
                 capabilities supported by the endpoint.

                 Received AVDTP_GET_CAPABILITIES_CNF
                         AVDTP Handle:
                             ADDR: E2:E2:A7:B8:70:E4
                             Remote SEID = 0x01
                         Result = 0x0000
                         Event Data = 2022A634, Data Length = 10
                         Remote SEP Capabilities:
                             Media Type = 0x00 -> Audio
                             Codec Type = 0x00 -> SBC
                             Codec IE   = 0xFF 0xFF 0x02 0xFA
                         Code Type: SBC
                         SBC Codec Capability IE = 0xFF 0xFF 0x02 0xFA
                             -> Sampling Frequency = 16000 32000 44100 48000
                             -> Channel Modes = Mono Dual Stereo Joint-Stereo
                             -> Block Length = 4 8 12 16
                             -> Subbands = 4 8
                             -> Allocation Method = SNR Loudness
                             -> Bitpool Range = 2 - 250
          6b). Get all the capabilities of the required endpoint ID as displayed in the Discover response
               - Enter option 13 (AVDTP Get All Capabilities) in A2DP menu
                 > Enter the remote Stream Endpoint Identifier (SEID), 1 (ACP SEID) in this case is the Sink endpoint as in response above
               - On successful procedure completion, the following prints should be on the terminal. This displays the codec type and
                 capabilities supported by the endpoint.

                 Received AVDTP_GET_ALL_CAPABILITIES_CNF
                         AVDTP Handle:
                             ADDR: 3F:E3:08:DC:1B:00
                             Remote SEID = 0x01
                         Result = 0x0000
                         Event Data = 202434B4, Data Length = 12
                         Remote SEP Capabilities:
                             Media Type = 0x00 -> Audio
                             Codec Type = 0x00 -> SBC
                             Codec IE   = 0xFF 0xFF 0x02 0xFA
                         Code Type: SBC
                         SBC Codec Capability IE = 0xFF 0xFF 0x02 0xFA
                             -> Sampling Frequency = 16000 32000 44100 48000
                             -> Channel Modes = Mono Dual Stereo Joint-Stereo
                             -> Block Length = 4 8 12 16
                             -> Subbands = 4 8
                             -> Allocation Method = SNR Loudness
                             -> Bitpool Range = 2 - 250
                         Delay Reporting Service: YES

               - Note that the aboove procedure is required in order to configure Delay reporting in the A2DP configuration in case the Sink supports.
          7). Create the A2DP connection with the required endpoint
              - Enter option 5 (A2DP Connect) in A2DP menu
                > Enter A2DP instance as source, 0
                > Enter the ACL connection index, 0
                > Default configuration parameters are set by default that correspond to the preloaded music array.
                > To enable dynamic configuration at runtime, disable the flag APPL_A2DP_DEFAULT_CONFIG in the appl_a2dp.c file.
                > Enter the remote SEID, 1 in this case as in response above
              - On successful connection, the following print will be displayed on the terminal.

                Received A2DP_CONNECT_CNF
                        Codec Instance = 0
                        Event Result   = 0x0000

          8). Start A2DP Streaming
              - Enter option 6 (A2DP Start) in A2DP menu
                > Enter A2DP instance as source, 0
              - On successful streaming start procedure, the following print will be displayed on the terminal. And the music should be heard on the
                sink device

                Received A2DP_START_CNF
                        Codec Instance = 0
                        Event Result   = 0x0000

          9). Suspend A2DP Streaming
              - Enter option 7 (A2DP Suspend) in A2DP menu
                > Enter A2DP instance as source, 0
              - On successful streaming suspend procedure, the following print will be displayed on the terminal. And the music should have stopped on the
                sink device

                Received A2DP_SUSPEND_CNF
                        Codec Instance = 0
                        Event Result   = 0x0000

          10). Disconnect A2DP Connection
               - Enter option 9 (A2DP Disconnect) in A2DP menu
                 > Enter A2DP instance as source, 0
               - On successful disconnection procedure, the following print will be displayed on the terminal.

                 Received A2DP_DISCONNECT_CNF
                         Codec Instance = 0
                         Event Result   = 0x0000

          11).Disconnect AVDTP transport
              - Enter option 14 (AVDTP Disconnect) in A2DP menu
                > Enter the ACL connection index, 0
              - On successful AVDTP transport disconnection, the following prints should be on the terminal.

                Received AVDTP_DISCONNECT_CNF
                        AVDTP Handle:
                            ADDR: E2:E2:A7:B8:70:E4
                        Result = 0x0000

          NOTE: Typically headsets connect for A2DP immediately after AVDTP transport connection. In case of such an event, this reference application requires to disconnect A2DP and then reconnect A2DP from local side for proper configuration of media information for SBC, or do a Reconfigure of the link for the default supported Sampling Rate at the Source. From the step 8), do as follow:

          8). Reconfigure the A2DP Channel
             - Enter option 8 (A2DP Reconfigure) in A2DP menu
               > Select A2DP instance, 0

          9). To disconnect the A2DP connection, select option 9 followed by the A2DP instance.
          10). To disconnect the AVDTP connection, select option 14.

          Default Streaming Configuration
          --------------------------------
          The default configuration of the A2DP Source reference application is to stream a 16KHz stereo sample. In order to enable 48KHz stereo configuration, enable the flag A2DP_PL_SOURCE_48KHz_MEDIA in a2dp_pl.h. Also place a 48KHz stereo file in WAV format in the name 'sample.wav' under 'data/a2dp' in the USB memory drive and have it plugged in to the RT1060.

          Streaming Procedure
          0). To start the A2DP streaming procedure, select option 6 followed by the A2DP instance
          1). To suspend the A2DP streaming procedure, select option 7 followed by the A2DP instance

          MTU and Channel Parameters
          ---------------------------
           0. To update the A2DP sink media MTU size, select option 20 and give the size.
              Please note that this can be done only when the A2DP is not in connected state.

          TROUBLESHOOTING
          ----------------
          1. Sometimes when initiating connection to headset, after AVDTP transport connection is estabished, the A2DP connection might return following failures:
           - 0x6614 (A2DP_INVALID_STATE): Invalid State
              - In case of this error, please check if the A2DP_CONNECT_IND is already received in the application console. This means that after the AVDTP transport connection, the headset has setup the A2DP connection already. Try an A2DP Disconnect and then Connect again.
           - 0x6615 (A2DP_NO_MATCHING_CODEC_FOUND): No Free/Matching Codec
              - In case of this error, the stack is unable to find a free codec instance for connection or a codec instance with matching capabilities at the headset.
           - 0x0129 (AVDTP_UNSUPPORTED_CONFIGURATION): Unsupported Configuration
              - This happens when the requested configuration is not supported by the headset. The current Sampling Frequency supported by the A2DP Source appliation is 16KHz Stereo. Since many of the headsets support only 44.1 and 48KHz configuration, they might return error to this 16KHz configuration. The application can be enabled to try different combinations of user given configurations by disabling the flag APPL_A2DP_DEFAULT_CONFIG in the appl_a2dp.c application file.
          2. SBC encoder error code:
           - 0x1104 (SBC_MAX_BITRATE_EXCEED): 
              - The A2dp profile limits the available maximum bit rate to 320kb/s for mono, and 512kb/s for two-channel modes. If the bit rate that is calculated based on sample rate, channel mode, block length, subband and bitpool exceeds the limitation, this error code is return.
       b) Running Advanced Audio Distribution Profile - Sink
          1). Setup and prepare the application.
              - Enter option 2 (EtherMind Init) in Main menu.
              - Enter option 3 (Bluetooth ON) in Main menu.
          2). Setup the A2DP profile
              - Enter option 65 (A2DP Operations) in Main menu.
              - Enter option 2 (A2DP Initialize) in A2DP menu
              - Enter option 4 (A2DP Register Codec) in A2DP menu
                > Enter Sink Endpoint; 1
                > Enter codec type SBC; 0
          3a). Discover the device from a peer mobile phone and connect to it. The following prints should be displayed on the terminal.
         
              Received SM Service UI Notification. Event Type 0x01
              Received UI Connection Request from SM
              ADDR: F1:CE:D4:F9:A2:64
              Replying to UI Connection Request ... OK
              Received HCI_COMMAND_STATUS_EVENT.
                      Command Status: 0x00
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x0409 (HCI_ACCEPT_CONNECTION_REQUEST_OPCODE)

              Received HCI_COMMAND_STATUS_EVENT.
                      Command Status: 0x00
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x0409 (UNKNOWN)
              Received HCI_PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT.
                      BD_ADDR:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

                      Page Scan Repetition Mode: 0x01

              Unknown Event Code 0x20 Received.
              Received HCI_CONNECTION_COMPLETE_EVENT.
                      Status: 0x00
                      Connection Handle: 0x0001
                      BD_ADDR:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

                      Link Type: 0x01 -> ACL
                      Ecnryption Mode: 0x00 -> Encryption OFF

              Unknown Event Code 0x03 Received.
              Unknown Event Code 0x38 Received.

              Unknown Event Code 0x38 Received.
              Received HCI_MAX_SLOTS_CHANGE_EVENT.
                      Connection Handle: 0x0001
                      LMP Max Slots: 0x05

              Unknown Event Code 0x1B Received.
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x042B -> UNKNOWN
                      Command Status: 0x00
                      Return Parameters:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x042B -> UNKNOWN
                      Command Status: 0x00
                      Return Parameters:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

              Received SM Service UI Notification. Event Type 0x06
              Received UI User Conf Request from SM
              ADDR: F1:CE:D4:F9:A2:64
              Numeric Value = 528999 (0x00081267)
              Replying to UI User Conf Request ... OK
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x042C -> UNKNOWN
                      Command Status: 0x00
                      Return Parameters:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x042C -> UNKNOWN
                      Command Status: 0x00
                      Return Parameters:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

              Unknown Event Code 0x36 Received.

              Unknown Event Code 0x36 Received.
              Received HCI_LINK_KEY_NOTIFICATION_EVENT.
                      BD_ADDR:
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              F1 CE D4 F9 A2 64                                  .....d
              -------------------------------------------------------------------

                      Link Key:
              -- Dumping 16 Bytes --
              -------------------------------------------------------------------
              40 2E CD 89 06 EE 04 F2 B2 76 D4 0A 9D CD 10 62    @........v.....b
              -------------------------------------------------------------------

                      Key Type: 0x05 -> ???

              Unknown Event Code 0x18 Received.
              Received HCI_ENCRYPTION_CHANGE_EVENT.
                      Status: 0x00
                      Connection Handle: 0x0001
                      Ecnryption Enable: 0x01 -> Encryption ON

              Unknown Event Code 0x08 Received.
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x1408 -> HCI_READ_ENCRYPTION_KEY_SIZE_OPCODE
                      Command Status: 0x00
                      Read Encryption Key Size Status: 0x00
                      Connection Handle: 0x0001
                      Key Size: 0x10

              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x1408 -> UNKNOWN
                      Command Status: 0x00
                      Return Parameters:
              -- Dumping 3 Bytes --
              -------------------------------------------------------------------
              01 00 10                                           ...
              -------------------------------------------------------------------

              Received AVDTP_CONNECT_IND
                      AVDTP Handle:
                          ADDR: F1:CE:D4:F9:A2:64
                      Result = 0x0000

              Received A2DP_CONFIGURE_IND
                      Codec Instance = 0
                      Event Result   = 0x0000
                      Event Data = 202421A4, Data Length = 12
              [APPL] A2DP Sink Configured
              Code Type: SBC
                      SBC Codec IE = 0x21 0x15 0x02 0x35
                      -> Sampling Frequency = 44100
                      -> Channel Mode = Joint Stereo (2)
                      -> Block Length = 16
                      -> Subbands = 8
                      -> Allocation Method = Loudness
                      -> Bitpool = 2, 53
               Delay Report Support: NO

              Received A2DP_CONNECT_IND
                      Codec Instance = 0
                      Event Result   = 0x0000

          MTU and Channel Parameters
          ---------------------------
           0. To update the A2DP sink media MTU size, select option 20 and give the size.
              Please note that this can be done only when the A2DP is not in connected state.
           1. To get the current Media Channel parameters like CID and MTU that is configured,
              select option 21. This is a supportive interface that can be used by the
              application in case of platforms where if the controller is capable to handle
              media without involving the host, it would require these information.
         3b). Initiate a connection to a peer A2DP Source
            0. Make the A2DP Source in discoverable mode, search and create and ACL connection as desribed in the 'readme_main.txt'

            1. Create and AVDTP transport connection as below
                - Enter option 10 (AVDTP Connect) in A2DP menu
                > Enter the ACL connection index, 0
                - On successful AVDTP transport connection, the following prints should be on the terminal.

                Received AVDTP_CONNECT_CNF
                    AVDTP Handle:
                        ADDR: E2:E2:A7:B8:70:E4
                    Result = 0x0000

             2. Discover the Endpoints in the peer device
                - Enter option 11 (AVDTP Discover) in A2DP menu
                  > Enter the ACL connection index, 0
                - On successful discovery completion, the following prints should be on the terminal.

                Received AVDTP_DISCOVER_CNF
                        AVDTP Handle:
                            ADDR: E2:E2:A7:B8:70:E4
                            Remote SEID = 0x00
                        Result = 0x0000
                        Event Data = 2022A668, Data Length = 2
                Response Buf [0] = 0x04, [1] = 0x08
                        Remote SEP Information [0]:
                            In Use     = No
                            SEP Type   = 0x01 -> Source
                            ACP SEID   = 0x01
                            Media Type = 0x00 -> Audio

             3. Get the capabilities of the required endpoint ID as displayed in the Discover response
                - Enter option 12 (AVDTP Get Capabilities) in A2DP menu
                  > Enter the ACL connection index, 0
                  > Enter the remote Stream Endpoint Identifier (SEID), 1 (ACP SEID) in this case is the Sink endpoint as in response above
                - On successful procedure completion, the following prints should be on the terminal. This displays the codec type and
                  capabilities supported by the endpoint.

                Received AVDTP_GET_CAPABILITIES_CNF
                        AVDTP Handle:
                            ADDR: E2:E2:A7:B8:70:E4
                            Remote SEID = 0x01
                        Result = 0x0000
                        Event Data = 2022A634, Data Length = 10
                        Remote SEP Capabilities:
                            Media Type = 0x00 -> Audio
                            Codec Type = 0x00 -> SBC
                            Codec IE   = 0xFF 0xFF 0x02 0xFA
                        Code Type: SBC
                        SBC Codec Capability IE = 0xFF 0xFF 0x02 0xFA
                            -> Sampling Frequency = 16000 32000 44100 48000
                            -> Channel Modes = Mono Dual Stereo Joint-Stereo
                            -> Block Length = 4 8 12 16
                            -> Subbands = 4 8
                            -> Allocation Method = SNR Loudness
                            -> Bitpool Range = 2 - 250

             4. Create the A2DP connection with the required endpoint
                - Enter option 5 (A2DP Connect) in A2DP menu
                  > Enter A2DP instance as source, 0
                  > Enter the ACL connection index, 0
                  > Default configuration parameters are set by default that correspond to the preloaded music array.
                  > To enable dynamic configuration at runtime, disable the flag APPL_A2DP_DEFAULT_CONFIG in the appl_a2dp.c file.
                  > Enter the remote SEID, 1 in this case as in response above
                - On successful connection, the following print will be displayed on the terminal.

                Received A2DP_CONNECT_CNF
                        Codec Instance = 0
                        Event Result   = 0x0000


            Streaming Procedure
            --------------------
             0. To start the A2DP streaming procedure, select option 6 followed by the A2DP instance
             1. To suspend the A2DP streaming procedure, select option 7 followed by the A2DP instance
            Disconnection Procedure
            -----------------------
             3. To disconnect the A2DP connection, select option 9 followed by the A2DP instance.
             4. To disconnect the AVDTP connection, select option 14.
            Delay Reporting
            ----------------
            0. To send the current delay report value to the source, select option 16 and enter the delay.
            1. To set an initial delay report value after confirming that A2DP Source offers Delay Reporting with AVDTP GetAllCapabilities procedure,
               select option 15 and enter the delay.

            TROUBLESHOOTING
            ----------------
            1. Sometimes when initiating connection to A2DP Source, after AVDTP transport connection is estabished, the A2DP connection might return following failures:
             - 0x6614 (A2DP_INVALID_STATE): Invalid State
               - In case of this error, please check if the A2DP_CONNECT_IND is already received in the application console. This means that after the AVDTP transport connection, the A2DP Source has setup the A2DP connection already. Try an A2DP Disconnect and then Connect again.
             - 0x6615 (A2DP_NO_MATCHING_CODEC_FOUND): No Free/Matching Codec
               - In case of this error, the stack is unable to find a free codec instance for connection or a codec instance with matching capabilities at the A2DP Source.
             - 0x0129 (AVDTP_UNSUPPORTED_CONFIGURATION): Unsupported Configuration
               - This happens when the requested configuration is not supported by the A2DP Source. The application can be enabled to try different combinations of user given configurations by disabling the flag APPL_A2DP_DEFAULT_CONFIG in the appl_a2dp.c application file.

            2. A2DP Streaming does not trigger a media player to start playing music on the A2DP Source device. The user should manually play the music on the media player in the A2DP source device.

            3. Sending of delay reports to certain A2DP source devices which do not support the feature or have not configured the sink for delay report indications will result in a failure with the error code 0x661E (A2DP_DELAY_REPORT_SEND_FAILED). The application can initiate an AVDTP_GET_ALL_CAPABILITIES procedure (option 13 in A2DP menu) to know if the peer supports Delay Reporting.
            
            In case the support for delay report is offered by the peer A2DP Source in the the GetAllCapabilities response, the application can disconnect and reconnect only the A2DP Connection (AVDTP need not be disconnected) after setting an initial delay report value with option 15. The following would be the procedure.
             0. A2DP Connected from Source with Delay Reporting not configured
             1. Perform AVDTP GetAllCapabilities procedure on the corresponding SEID of peer
             2. Check if DelayReporting supported. If yes, and the application wants to enable delay reporting, Set the initial delay (option 15) with the required delay value.
             3. Disconnect the A2DP (Option 9)
             4. Connect the A2DP (Option 5). The application should see a A2DP_DELAY_REPORT_CNF event as a result of sending the first delay upon configuration.
             5. Now sending delay reports with option 16 should work as expected.

            LIMITION
            ----------------
            there is a short noise at the begin audio streaming.The codec power on pop noise cannot eliminate.
            
            Notice: Headset is connected to J12 of RT1060 board.
            
    8.4 Running Sample LE HPS Application
       0). Projects and Profile/Service Feature Flag Update.
           # The project to use this Profile Service application is evkmimxrt1060_ethermind.
           # Enable the macro "HPS" in evkmimxrt1060_ethermind\ethermind\bluetooth\export\appl\reference\le\profile_appl\server\gatt_db.h
             and disable any other Profile/Services macro in project compiler preprocessor setting.
           # Make sure that the feature flag "APPL_MENU_OPS" is defined in the project compiler preprocessor setting.
           # The Ethernet port should connect to a useable network with network cable.

       1). Refer to main menu 1-7 steps to setup and prepare the application.
       
       2). This demo HPS application has GAP Peripheral roles defined by default. But, the HPS needs to play the role of GAP Central.
           On Bluetooth Initialization Procedure, following USER Prompt will appear on the Console.
                                      NOTE to USER
                                      ----------
              Default GAP Role is GAP Central for this HPS Service. But, during
              init time setting the role to Peripheral. This needs to be updated
              from the HPS Menu after the desired Peer Device Address to which this
              service needs to scan and auto initiate connection is Set.

                      ** User needs to select the role from the HPS Menu **

       3). Select GATT server Menu by selecting "Option 81". And navigate to Service Menu Operations by selecting "Option 10".
       
       4). Now, choose HPS Menu Operations by selecting "Option 24". The following menu should be seen on the console:

              0. Exit
              1. Refresh
              10. Set Network Availability
              11. Update Peer BD Address to Auto-Connect
              12. Set GAP Central Role
              Your Option ?

       5). Typically the HPS application needs to act as a GAP Central device. For this, first Set the Peer BD Address that is needed to connect through
           the option 11.

              Enter the Peer BD Address to be updated and used...
              Enter BD_ADDR : 55 44 33 22 11 00
              Enter bd_addr_type : 0

              Default Config Peer Address Updated to :- ADDR: 55:44:33:22:11:00, TYPE: 00

       6). Now, select the GAP role as "Central" from the "Option 12 - Set GAP Central Role".
       
       7). Now, the HPS device should automatically scan and connect to the desired Peer BD Address.
       
       8). After this the Peer can perform various operations such as Discovery of HPS service and its characterisitcs, perform HTTP Methods like
           GET/HEAD/POST/PUT/DELETE/CANCEL etc over HPS Control Point etc.
       
       9). This HPS application will establish connection over LWIP on each of the Control Point procedures and send HTTP Status Notifications to the remote
           HPS Collector/Client application.
       
       10). Use "Option 10" to simulate the scenario of "Network Non-Availability". When this option is selected, below menu will appear
              
              Set Network Availability State
              0.No Network
              1.Network Available

            NOTE: This specific option would be needed for some test scenarios with Bluetooth Profile Tuning Suite (PTS ver 7.6.2) etc.
       
       11). For example, When the Remote HPS Client/Collector application performs a HTTP GET Request Method from the HPS Control Point, the below
            logs should be see on the console. Here the HPS server will
            - Get the IP address from the Host name provided by the HPS Client/Collector.
            - Establish a successful TCP-IP connection to this IP address. [Default Port is 80]
            - frame the HTTP METHOD command depending on the control point opcode and send the Command
            - Send Write response to the Control point request.
            - Receive the data from the Remote HTTP Server over the Network
            - Send the HTTP Status Notification to the Remote HPS Client/Collector.

            Trying to resolve Host name: httpbin.org
            Host name: httpbin.org resolved to: 18.208.255.250
            HPS Transport Established Successfully!
            Constructed HTTP Request:

            GET http://httpbin.org/get HTTP/1.1
            Host:httpbin.org


            [APPL]: Write Response sent with result 0x0000
            Received HTTP Response is:
            HTTP/1.1 200 OK
            Date: Tue, 27 Oct 2020 21:39:14 GMT
            Content-Type: application/json
            Content-Length: 199
            Connection: keep-alive
            Server: gunicorn/19.9.0
            Access-Control-Allow-Origin: *
            Access-Control-Allow-Credentials: true

            {
              "args": {},
              "headers": {
                "Host": "httpbin.org",
                "X-Amzn-Trace-Id": "Root=1-5f989382-4bdd8d0e76b4f6684cd36be5"
              },
              "origin": "122.172.219.58",
              "url": "http://httpbin.org/get"
            }


            Received HTTP Status Line of length 15 is:
            HTTP/1.1 200 OK

            Received HTTP Response Header is:
            Date: Tue, 27 Oct 2020 21:39:14 GMT
            Content-Type: application/json
            Content-Length: 199
            Connection: keep-alive
            Server: gunicorn/19.9.0
            Access-Control-Allow-Origin: *
            Access-Control-Allow-Credentials: true


            Received HTTP Response Entity Body is:
            {
              "args": {},
              "headers": {
                "Host": "httpbin.org",
                "X-Amzn-Trace-Id": "Root=1-5f989382-4bdd8d0e76b4f6684cd36be5"
              },
              "origin": "122.172.219.58",
              "url": "http://httpbin.org/get"
            }

            [APPL]: Stopping Control Point Procedure Timer 2022F34C, result 0x0000!
            [0x02]:Sending http status code 200 On Handle 0x0028
            [ATT]:[0x0D: 0x01]: Received ATT Event 0x1C with result 0x0000

            -- Dumping 5 Bytes --
            -------------------------------------------------------------------
            28 00 C8 00 05                                     (....
            -------------------------------------------------------------------

            [ATT]:[0x0D]: Received event 0x1C with result 0x0000
            Received HVN Tx Complete (Locally generated)

            -- Dumping 5 Bytes --
            -------------------------------------------------------------------
            28 00 C8 00 05                                     (....
            -------------------------------------------------------------------
        
        NOTE:
            1). This demo application currently does not handle scenarios of HTTP Certificate verification for the URI requested by the Peer HPS Client application.
                Currently, This demo uses LwIP to realize the TCP/IP functionality.
                To realize HTTPS functionality, we need HTTP+SSL/TLS. SSL/TLS enables the encryption procedures.
                LwIP Module inherently does not expose interfaces for SSL/TLS.
                Need to explore other modules from the SDK which exposes the needed interfaces to realize HTTPS functionality.

            2). This demo use fixed MAC address 02:12:13:10:15:11 for Ethernet connection.
                If need use two HPS demo boards under one router at the same time, please change the MAC address to prevent MAC conflacts.
                (the fixed MAC address located: ethermind\port\pal\mcux\bluetooth\niface.c line 32)

    8.5 Running Sample LE Proximity Application
       a) Running Proximity Profile - GATT Server
          0). Projects and Profile/Service Feature Flag Update.
              # The project to use this Profile Service application is evkmimxrt1060_ethermind.
              # Enable the macro "PXR" in evkmimxrt1060_ethermind\ethermind\bluetooth\export\appl\reference\le\profile_appl\server\gatt_db.h
                and disable any other Profile/Services macro in project compiler preprocessor setting.
          
          1). Refer to main menu 1-7 steps to setup and prepare the application.
          
          2). This demo PXR application plays GAP Peripheral roles. Hence, by defaut this demo application will advertise once BT ON Initialization sequence is
              completed.
          
          3). After the "Bluetooth ON Initialization Completed" is successfully completed. The Demo application
              automatically sets the BLE Advertisement Parameters, Data and enables Advertisements.
              The following prints should be seen on the terminal
              
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x0C6D -> UNKNOWN
                      Command Status: 0x00
              
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2008 -> HCI_LE_SET_ADVERTISING_DATA_OPCODE
                      Command Status: 0x00
              
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2006 -> HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE
                      Command Status: 0x00
              
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x200A -> HCI_LE_SET_ADVERTISING_ENABLE_OPCODE
                      Command Status: 0x00
              Enabled Advertising...
              EtherMind Server is now Ready
          
          4). Now, scan for this device from a Mobile device, For example from an Android 10 device which runs nRF Connect App.
          
          5). On the app, "Mt-PXR" should be seen. From the app it should now be possible to Connect, Discover Services, Read Characterisitics
              and Enable Notifications/Indications of Characterisitics that supports it.
              This Demo application consist of the following Services:
              # Generic Access
              # Generic Attribute
              # Battery Service
              # Link Loss
              # Immediate Alert
              # Tx Power
          
          6). The Demo is configured with the IO Capability set to "Display Only". When the user initiates "Bond" from the mobile App, look for the
              below prints on the terminal where the PASSKEY to be used for the Bonding/Pairing Session can be found
              
              Event   : SMP_PASSKEY_DISPLAY_REQUEST
              BD Address : EA 5F E8 3F 56 61
              BD addr type : Random Address
              Passkey : 491152
          
          7). When the Bonding/Pairing is successfully completed, the following print should be seen on the terminal
              Recvd SMP_AUTHENTICATION_COMPLETE
              BD Address : EA 5F E8 3F 56 61
              BD addr type : Random Address
              Status : 0000
              Authentication type : With MITM
              Pairing Mode : LEGACY Pairing Mode
              Bonding type : Bonding
              Encryption Key size : 16
          
          8). When the Peer Client(Proximity monitor) i.e. the Phone app configures the Immediate Alert Level. Depending on the level of the alert, following
              should be seen in the terminal (below example is for HIGH Alert Level)
                                      User Alert
                                      ----------
              [Proximity]: HIGH Alert => IMMEDIATE ALERT
          
          9). When the Peer Client(Proximity monitor) i.e. the Phone app configures the Link Loss Alert Level. Depending on the level of the alert, following
              should be seen in the terminal once the Link Disconnection happens (below example is for HIGH Alert Level)
                                      User Alert
                                      ----------
              [Proximity]: HIGH Alert => LINK LOST
          
          10). The Peer Client(Proximity monitor) i.e. the Phone app can configure the TX Power Level for Notifications. When the Notifications are enabled,
               the Demo tries to fetch the Transmit power level from the BT Controller through the HCI command and Notifies the TX Power Level every
               "APPL_PXR_TPS_MSRMT_TIMEOUT" seconds. This value would typically be configured to 5 seconds as default.

       b) Running Proximity Profile - GATT Client
          0). Projects and Profile/Service Feature Flag Update
              # The project to use this Profile-Collector/Service application is evkmimxrt1060_ethermind.
              # Since this is a Collector application, there is no specific Feature flag needed to be enabled in either preprocessor settings or in any Header file.
                By default "PXM" flag should be enabled in the project settings.

          1). Refer to main menu 1-7 steps to setup and prepare the application.
          
          2). The application will automatically start to Advertise after Bluetooth Initialization Procedures irrespective of the Service Specific Feature flag
              defined in gatt_db.h.
          
          3). The PXM collector needs to play the role of the GAP Central Device and GATT Client Device.
          
          4). If the device is already advertising, go to "GATT Client Menu" by selecting "Option 80" from Main Menu.
          
          5). Set the desired "Peer Device Address" to which Connection has to be initiated in Selecting "Option 3".
              NOTE: BD Address is entered from LSB to MSB and separated with space. i.e. if BD address is
              0x001122334455, enter the address as 55 44 33 22 11 00. Please refer to the below prints:

              Enter the Peer BD Address to be updated and used...
              Enter BD_ADDR : 55 44 33 22 11 00
              Enter bd_addr_type : 0
              Default Config Peer Address Updated to :- ADDR: 55:44:33:22:11:00, TYPE: 00
          
          6). After this set the GAP role for the current scenario by selecting "Option 4". In this case the GAP role is CENTRAL.
              
              Enter you choice : 4
              Set Current GAP Role
              0.GAP Central
              1.GAP Peripheral
              0
              Default GAP Role Set to CENTRAL
          
          7). Once the application receives a connectable ADV report from this configured peer device address. It will automatically initiate an LE Connection.
          
          8). Once, the LE connection is established successfully, you will see the below prints
          
Received HCI_LE_META_EVENT.
              Subevent : HCI_LE_CONNECTION_COMPLETE_SUBEVENT.
              status = 0x00
              connection_handle = 0x005D
              role = 0x00
              peer_addr_type = 0x00
              peer_addr =
              
              -- Dumping 6 Bytes --
              -------------------------------------------------------------------
              55 44 33 22 11 00                                  @.....
              -------------------------------------------------------------------
              
              conn_interval = 0x0006
              conn_latency = 0x0000
              supervision_timeout = 0x03BB
              clock_accuracy = 0x00
          
              NOTE: Please ignore any prints such as
                - ACL Connected with IPSP Node: This is because IPSP Router might also be enabled in the project.
                - Unknown Event Code 0x3E Received: This is recevied from the BREDR HCI callback which does not parse this LE META event.
          
          9). After the successful LE connection navigate to PXM Client operations menu by selecting "Option 100 - Profile Client Menu" and
              finally selct PXM Client by selecting "Option 140 - PX Profile Operation". PXM menu shows below options
              
                0 - Exit
                1 - Refresh
           
               --- Link Loss Service ---
               10 - Discover Link Loss Service
               11 - Read Link Loss Service - Alert Level
               12 - Set Link Loss Service - Alert Level - "No Alert"
               13 - Set Link Loss Service - Alert Level - "Mild Alert"
               14 - Set Link Loss Service - Alert Level - "High Alert"
           
               --- Immediate Alert Service ---
               20 - Discover Immediate Alert Service
               21 - Immediate Alert Service - Alert Level - "No Alert"
               22 - Immediate Alert Service - Alert Level - "Mild Alert"
               23 - Immediate Alert Service - Alert Level - "High Alert"
           
               --- Tx Power Service ---
               30 - Discover Tx Power Service
               31 - Read Tx Power Level
               32 - Enable/Disable TX Power Service Notification
               Your Option?
          
          10). Using different options from this menu, one can control the Link Loss Service, Immediate Alert Service and Tx Power Service on the remote Proximity
               Reporter Device.
          
          11). To Initiate Disconnection with the currently connected device. Go back to the "GATT Client Menu" and choose "Option 8"
          
          12). To Initiate Pairing with the remote device, Go back to Main Menu and navigate to "SMP Menu" by selecting "Option 31".
               - Here choose the "Option 2 : Authenticate Remote Device".
               - Use the below parameters to Initiate an SMP Pairing Procedures with the remote device with
                 # SMP LE Secure Connections
                 # MITM Enabled
                 # Bonding Set
                 # Key Size 16
           
               Enter Your Choice: 2
               0 - Legacy LE SMP Procedure
               1 - LE Secure Connections Mode
               Enter SMP Pairing Mode :1
               0 - Encryption Only/Unauthenticated (Without MITM)
               1 - Authenticated (With MITM)
               Enter Security level required : 1
               0 - non-Bonding
               1 - Bonding
               Enter Bonding type : 1
               Enter Encryption Key size required : 16

    8.6 Running Sample LE HTS Application
       0). Projects and Profile/Service Feature Flag Update.
           # The project to use this Profile Service application is evkmimxrt1060_ethermind.
           # Enable the macro "HTS" in evkmimxrt1060_ethermind\ethermind\bluetooth\export\appl\reference\le\profile_appl\server\gatt_db.h
             and disable any other Profile/Services macro in project compiler preprocessor setting.
       
       1). Refer to main menu 1-7 steps to setup and prepare the application.
       
       2). This demo HTS application plays GAP Peripheral roles. Hence, by defaut this demo application will advertise once BT ON Initialization sequence is
          completed.
       
       3). After the "Bluetooth ON Initialization Completed" is successfully completed. The Demo application
          automatically sets the BLE Advertisement Parameters, Data and enables Advertisements.
          The following prints should be seen on the terminal
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x0C6D -> UNKNOWN
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2008 -> HCI_LE_SET_ADVERTISING_DATA_OPCODE
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2006 -> HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x200A -> HCI_LE_SET_ADVERTISING_ENABLE_OPCODE
                      Command Status: 0x00
              Enabled Advertising...
              EtherMind Server is now Ready
      
       4). Now, scan for this device from a Mobile device, For example from an Android 10 device which runs nRF Connect App.
       
       5). On the app, "Mt-HTS" should be seen. From the app it should now be possible to Connect, Discover Services, Read Characterisitics
          and Enable Notifications/Indications of Characterisitics that supports it.
          This Demo application consist of the following Services:
           # Generic Access
           # Generic Attribute
           # Battery Service
           # Device Information
           # Health Thermometer
      
       6). The Demo is configured with the IO Capability set to "Display Only". When the user initiates "Bond" from the mobile App, look for the
          below prints on the terminal where the PASSKEY to be used for the Bonding/Pairing Session can be found
      
              Event   : SMP_PASSKEY_DISPLAY_REQUEST
              BD Address : EA 5F E8 3F 56 61
              BD addr type : Random Address
              Passkey : 491152
      
       7). When the Bonding/Pairing is successfully completed, the following print should be seen on the terminal
      
              Recvd SMP_AUTHENTICATION_COMPLETE
              BD Address : EA 5F E8 3F 56 61
              BD addr type : Random Address
              Status : 0000
              Authentication type : With MITM
              Pairing Mode : LEGACY Pairing Mode
              Bonding type : Bonding
              Encryption Key size : 16
      
       8). When the Peer Client(Health Thermometer Collector) i.e. the Phone app configures the Temperature Measurement for Indications, a default template
          temperature data is sent as Indications.
          The Template temperature corresponds to "36.4 C" with some default Time Stamp [29/04/2014 06:00:01 (DD/MM/YYYY H:M:S)].
          This data is sent as per the "Mesurement Interval" Characteristic Value. The default interval is 5 Seconds.
          This Demo supports a range of "Mesurement Interval" from 1 Second to 10 Seconds.
      
       9). When the Peer Client(Health Thermometer Collector) i.e. the Phone app also configures Intermediate temperaure measurement for Notifications,
          a series of default template temperature measurements are sent. These values are "35.9 C", "36.1 C" and "36.3 C".
       
       10). All the above default template temperature measurements are assigned the Temperature Type as "Armpit".
       
    8.7 Running Sample LE IPSPN Application
       0). Projects and Profile/Service Feature Flag Update.
           # The project to use this Profile Service application is evkmimxrt1060_ethermind.
           # Enable the macro "IPSPN" in evkmimxrt1060_ethermind\ethermind\bluetooth\export\appl\reference\le\profile_appl\server\gatt_db.h
             and disable any other Profile/Services macro in project compiler preprocessor setting.

       1). Refer to main menu 1-7 steps to setup and prepare the application.
       
       2). This demo IPSPN application plays GAP Peripheral roles. Hence, by defaut this demo application will advertise once BT ON Initialization sequence is
           completed.
       
       3). During the "Bluetooth ON Initialization Sequence", the application will automatically initialize LWIP stack and DHCP. Below prints will be seen
           on the Console
      
              Initializing PHY...
      
              lwip ping netif init complete!
      
              ************************************************
              DHCP start
              ************************************************
              DHCP state       : SELECTING
              DHCP state       : REQUESTING
              DHCP state       : CHECKING
              DHCP state       : BOUND
      
              IPv4 Address     : 192.168.1.144
              IPv4 Subnet mask : 255.255.255.0
              IPv4 Gateway     : 192.168.1.251
      
      
              lwip ping DHCP complete!
              SUCCESS!!
      
       4). After the "Bluetooth ON Initialization Completed" is successfully completed. The Demo application
          automatically sets the BLE Advertisement Parameters, Data and enables Advertisements.
          The following prints should be seen on the terminal
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x0C6D -> UNKNOWN
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2008 -> HCI_LE_SET_ADVERTISING_DATA_OPCODE
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x2006 -> HCI_LE_SET_ADVERTISING_PARAMETERS_OPCODE
                      Command Status: 0x00
      
              Received HCI_COMMAND_COMPLETE_EVENT.
                      Num Command Packets: 1 (0x01)
                      Command Opcode: 0x200A -> HCI_LE_SET_ADVERTISING_ENABLE_OPCODE
                      Command Status: 0x00
              Enabled Advertising...
              EtherMind Server is now Ready
      
       5). Now, scan for this device from a Mobile device, For example from an Android 10 device which runs nRF Connect App.
       
       6). On the app, "Mt-IPSPN" should be seen. From the app it should now be possible to Connect, Discover Services, Read Characterisitics
          and Enable Notifications/Indications of Characterisitics that supports it.
          This Demo applications consist of the following Services:
           # Generic Access
           # Generic Attribute
           # Battery Service
           # Internet Protocol Support
       
       7). The Demo is configured with the IO Capability set to "Display YesNo" and default SMP Pairing Mode set to SMP LE Secure connections.
           When the user initiates "Bond" from the mobile App, look for the below prints on the terminal where the Numeric Comparision Code to be
           used for the Bonding/Pairing Session can be found

            Event   : SMP_NUMERIC_KEY_COMPARISON_CNF_REQUEST
            BD Address : 68 00 48 DA 0C 67
            BD addr type : Random Address
            Numeric Code : 999999

       8). When the Bonding/Pairing is successfully completed, the following print should be seen on the terminal
            
            Recvd SMP_AUTHENTICATION_COMPLETE
            BD Address : 68 00 48 DA 0C 67
            BD addr type : Random Address
            Status : 0000
            Authentication type : With MITM
            Pairing Mode : LE SEC Pairing Mode
            Bonding type : Bonding
            Encryption Key size : 16

       9). This IPSPN Demo, is configured to accept LE L2CAP CoC request from remote device on LE IPSP PSM.
           The default MTU, MPS and Initial Credits value for this is 1500, 512 and 10.

       10). If IPSP_HAVE_6LO_NIFACE feature flag is defined, any value data received over this IPSP LE L2CAP CoC transport would be forwared to the Network
           stack or the 6LO Interface.

       11). This demo automatically handles the transfer of LE L2CAP CoC receive credits after each data packet reception if APPL_LE_L2CAP_AUTO_SEND_INSTANT_CREDIT.
           If APPL_LE_L2CAP_AUTO_SEND_CREDIT_ON_LWM is defined then credits are sent automatically, on reaching Low Water Mark.

          Notes:
          Please follow the below procedures to verify the "ping" over IPSPN between the
          EtherMind application running on iMXRT1060 and BlueZ stack on a Ubuntu Box.

          The following steps/commands not work on new Ubuntu/BlueZ version.
          So the Ubuntu version should be "Ubuntu 14.04" and BlueZ version should be "4.101".
          
          1). Run this IPSPN Demo and perform the Bluetooth ON Intializations and wait
              for LWIP and DHCP to start and the device to Advertise.
          2). On a Ubuntu machine (the version we used was 14.04LTS) which has the BlueZ
              stack installed, perform the following commands
              - Enter Super User mode
              - Setup a bluetooth device or use the machine's inbuilt BT controller and
                check for it to show on the hciconfig interface.
              - Initial Setup related commands [in Super User Mode]
           
              hciconfig hci0 up
                 modprobe bluetooth_6lowpan
                 echo 1 > /sys/kernel/debug/bluetooth/6lowpan_enable
           
              - BLE Connection related commands. Here, the device Bluetooth Address is
                D8C0A6C0B106(D8:C0:A6:C0:B1:06). Please update the address in below
                commands depending on the bluetooth address of the local device.
           
                echo "connect D8:C0:A6:C0:B1:06 1" > /sys/kernel/debug/bluetooth/6lowpan_control
          
          3). After the above steps, the BlueZ stack will initiate LE connection to the
              device and also establish an LE L2CAP CoC channel for IPSP PSM. The below
              prints should be seen on the device's teraterm/console. Also make a note
              of the LinkLocal address.
           
                BtLE 6Lo interface added successfully
                NetIF LinkLocal Address: FE80::DAC0:A6FF:FEC0:B106
                NetIF Global Address: 2001:db8::1

          4). On the Ubuntu machine, check if any bt interface is established by issuing
              "ifconfig" command. eg: typically bt0 interface would be created
              at this point.
          
          5). Based on the bt interface creation issue the below command an check if ping
              response is received. Please change the LinkLocal address based on the
              device being used and also the bt interface number based on the one that is
              created.
           
               ping6 -s 16 FE80::DAC0:A6FF:FEC0:B106%bt0
    
    8.8 Running Sample RFCOMM Application
       a) Running RFCOMM application as server
          1). Setup and prepare the application.
              - Enter option 2 (EtherMind Init) in Main menu.
              - Enter option 3 (Bluetooth ON) in Main menu.
          2). Setup the RFCOMM Server
              - Enter option 14 (RFCOMM Operations) in Main menu.
              - Enter option 4 (RFCOMM Channel Accept) in RFCOMM menu to setup server listening on default server channel 0x01.
       b) Running RFCOMM application as client
          1). Setup and prepare the application.
              - Enter option 2 (EtherMind Init) in Main menu.
              - Enter option 3 (Bluetooth ON) in Main menu.
          2). Discover the RFCOMM Server device and establish the ACL connection. 
              - Enter option 11 (HCI Operations) in Main menu
              - Enter option 10 (Create Connection (ACL)) in HCI menu
                > Enter the peer BD Address (in LSB to MSB order separated by space. Eg: 0x001BDCE0384C is the peer address,enter as "4C 38 E0 DC 1B 00")
                > Enter Clock Offset as 0
                > Enter Role Switch as 1
          3). Setup the RFCOMM Client
              - Enter option 0 (exit) in HCI menu.
              - Enter option 14 (RFCOMM Operations) in Main menu.
              - Enter option 2 (RFCOMM Channel Open) in RFCOMM menu.
                > Enter the RFCOMM Server BD Address (in LSB to MSB order separated by space. Eg: 0x001BDCE0384C is the peer address,enter as "4C 38 E0 DC 1B 00")
                > Default server channel 0x01 will be used since APPL_RFCOMM_READ_SERVER_CHANNEL flag is not defined.
       c) Write data on the connected RFCOMM channel
          - After RFCOMM connection is created, data can be sent from either server or client.
          - Enter option 11 (RFCOMM Channel Write) in RFCOMM menu
          - Enter the number of bytes to write
          - Enter the number of times to write
          - Data can be received by peer device
          
          Notice: If number of bytes to write is bigger than APPL_RFCOMM_DUMP_CHARS_MAX_LENGTH(8 as default),
                  receiver will just dump APPL_RFCOMM_DUMP_CHARS_MAX_LENGTH bytes. 
                  You can change this length in appl_rfcomm.h.

       d) File transfer on the connected RFCOMM channel
          - After RFCOMM connection is created, file transfer can be started from either server or client.
          - For receiver
            > Connect U-disk to RT1060 board's J9 with OTG
            > Enter option 12 (RFCOMM Receive File) in RFCOMM menu
            > Enter file name           
          - For sender
            > Save the file to be transferred in the U-disk
            > Connect U-disk to RT1060 board's J9 with OTG
            > Enter option 13 (RFCOMM Send File) in RFCOMM menu
            > Enter file name that is saved
       e) Close the connected RFCOMM channel
          - RFCOMM close can be operated from either server or client.
          - Enter option 3 (RFCOMM Channel Close)in RFCOMM menu.

 9. To set a device specific friendly name to be identified in a peer device during discovery.
    - Enter option 11 (HCI Operations) in Main menu.
    - Enter option 45 (Write Local Name) in HCI menu;
      > Enter a friendly name as desired.

 10.Disable the BLE Advertisement since the mobiles display the name in the advertisements sometimes.
    - Enter option 11 (HCI Operations) in Main menu.
    - Enter option 80 (LE Operations) in HCI menu.
    - Enter option 10 (Set advrt enable) in HCI BLE menu.
      > Enter advertising enable as 0

 11.If running any audio profiles like A2DP Sink/HFP Unit, it is required to set the Class of Device in order to enable the mobile phones
     to recognize this as a headset device and display during discovery.
    - Enter option 11 (HCI Operations) in Main menu.
    - Enter option 44 (Write Class of Device) in HCI menu;
      > Enter the value 200404 or 200408.

 13.To discover any peer device, set the inquiry mode and start the inquiry procedure as below
    - Enter option 11 (HCI Operations) in Main menu
    - Enter option 50 (Write Inquiry mode) in HCI menu
      > Enter inquiry mode as 2
    - Enter option 3 (Inquiry) in HCI menu
      > Enter LAP as 9E8B33
      > Enter Inquiry length as 10
      > Enter Num Responses as 10
    - The terminal should list discovered devices and the discovery process will end with the following event

Received HCI_INQUIRY_COMPLETE_EVENT.
        Status: 0x00

 12.To initiate a profile connection from the local device, start by establishing the base ACL link connection as below
    - Enter option 11 (HCI Operations) in Main menu
    - Enter option 10 (Create Connection (ACL));
      > Enter the peer BD Address (in LSB to MSB order separated by space. Eg: 0x001BDCE0384C is the peer address,enter as "4C 38 E0 DC 1B 00")
      > Enter Clock Offset as 0
      > Enter Role Switch as 1
    - The following prints will be displayed on successful connection initiation

Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0405 (HCI_CREATE_CONNECTION_OPCODE)

    -  On successful connection establishment, the following prints will be displayed.

Received HCI_CONNECTION_COMPLETE_EVENT.
        Status: 0x00
        Connection Handle: 0x0001
        BD_ADDR:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
F1 CE D4 F9 A2 64                                  .....d
-------------------------------------------------------------------

        Link Type: 0x01 -> ACL
        Ecnryption Mode: 0x00 -> Encryption OFF

Unknown Event Code 0x03 Received.
Received HCI_PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT.
        BD_ADDR:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
F1 CE D4 F9 A2 64                                  .....d
-------------------------------------------------------------------

        Page Scan Repetition Mode: 0x01

Unknown Event Code 0x20 Received.
Received HCI_MAX_SLOTS_CHANGE_EVENT.
        Connection Handle: 0x0001
        LMP Max Slots: 0x05

 13.To disconnect the ACL link with a connected peer device, do the following steps -
    - Enter option 11 (HCI Operations) in main menu
    - Enter option 20 (Get HCI Connection Details) in HCI menu. Note the connection handle of the connection.
    - Enter option 12 (Disconnection) in HCI menu;
      > Enter the connection handle as noted above
      > Enter reason as 13. 
    - The following prints will be displayed on successful disconnection initiation

Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0406 (HCI_DISCONNECT_OPCODE)

    -  On successful disconnection, the following prints will be displayed.

Received HCI_DISCONNECTION_COMPLETE_EVENT.
        Status: 0x00
        Connection Handle: 0x0001
        Reason: 0x16

 14.During the authentication procedure that gets triggered during any profile connection, the peer device like mobile phone will ask
    the confirmation of a 6 digit numeric key. This numeric can be looked up on the terminal for the local device with the following print.

Received SM Service UI Notification. Event Type 0x06
Received UI User Conf Request from SM
ADDR: F1:CE:D4:F9:A2:64
Numeric Value = 972886 (0x000ED856)
Replying to UI User Conf Request ... OK

 12.Upon successful Authentication and link Encyption, the following prints will be seen on the terminal

Received HCI_LINK_KEY_NOTIFICATION_EVENT.
        BD_ADDR:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
F1 CE D4 F9 A2 64                                  .....d
-------------------------------------------------------------------

        Link Key:
-- Dumping 16 Bytes --
-------------------------------------------------------------------
46 1C 72 D7 08 AE D6 B9 96 04 51 5D 0B 34 26 52    F.r.......Q].4&R
-------------------------------------------------------------------

        Key Type: 0x05 -> ???

Unknown Event Code 0x18 Received.
Received HCI_ENCRYPTION_CHANGE_EVENT.
        Status: 0x00
        Connection Handle: 0x0001
        Ecnryption Enable: 0x01 -> Encryption ON
Note:
Command 2(Set Security Mode) and 3(Get Security Mode) under SM option are not supported if BT_SSP flag is enabled in BT feature

    8.9 Wake on BLE and OOB wakeup over GPIO

        Make sure before starting build, preprocesor define "OOB_WAKEUP" is enabled.(by default enabled) 
        Host is RT1170 and Controller is Firecrest board.
        Host will initialize the RT1170 platform specific GPIOs used for Host to Controller and Controller to Host sleep and wakeup.
        Currently the bt_ble_cli application supports ISR trigger with print when controller indicates sleep and wakeup.
        So the application has to be modified when host has to go to sleep upon host sleep ISR.

        Vendor specific command 0x0053 is to enable/configure the Host to Controller and Controller to Host sleep and wakeup scenarios.

        - Enter option 250 in main menu
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0xFC53 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT in LE.
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0xFC53 -> UNKNOWN
        LE Command Status: 0x00

        Vendor specific command 0x0023 is to enable the Controller to make it ready for sleep and wakeup.

        - Enter option 251 in main menu
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0xFC23 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT in LE.
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0xFC23 -> UNKNOWN
        LE Command Status: 0x00

        Host to controller sleep and wakeup procedure:
        ----------------------------------------------

        Host will initialize controller sleep and wakeup mechanism through one time vendor specific commands 0x0053 and 0x0023 between power cycles.
        Host has to give 0x0023 command for controller SLEEP and WAKEUP to work.
        Make sure to give option 250 and 251 before using H2C GPIO Control Option 252 and 253.

	     For RT1170-EVKA, J9.12 shall be connected with J10.13 (GPIO18) of FC-RD Board for H2C trigger.
	     For RT1060-EVKA, J22.3 shall be connected with J9.8 (GPIO12) of uSD-M2 Adapter for 1XK/1ZM H2C trigger.
	     For RT1060-EVKB, J16.3 shall be connected with J9.8 (GPIO12)of uSD-M2 Adapter for 1XK/1ZM H2C trigger.

        Host will make H2C line high using option-252 to move controller into DEEPSLEEP mode. As soon as Controller move into DEEPSLEEP mode, Host will not be able to send any HCI command.
        Host will make H2C line low using option-253 to move controller into WAKEUP mode.

        Controller to Host sleep and wakeup procedure:
        ----------------------------------------------
        Controller will use C2H GPIO to indicate that host can be sleep/wakeup. To connect C2H Line,

	     For RT1170-EVKA, J9.6 shall be connected with HD3.16 (GPIO19) of FC-RD Board for H2C trigger.
	     For RT1060-EVKA, J33.1 shall be connected with J9.6 (1XK:GPIO14, 1ZM:GPIO20) of uSD-M2 Adapter for 1XK/1ZM H2C trigger.
	     For RT1060-EVKB, J23.1 shall be connected with J9.6 (1XK:GPIO14, 1ZM:GPIO20) of uSD-M2 Adapter for 1XK/1ZM H2C trigger.

	     Controller indication received to the host whenever there is high pulse.
	     Whenever there is C2H Interrupt received on host, "Host can now go to sleep" log will be appear on console.
        Host should sleep as soon as above C2H interrupt received using option-254.
        Host will wakeup as soon as C2H goes LOW (this happens when there is any event pending on host side which controller is sending).
        
        In this case it satisfies Wake on BLE triggered internally in controller wakes up Host externally using OOB GPIO.
	     Note: For 1XK/1ZM, C2H Support in Host is disabled for existing controller's issue.

        Switch 7 can be used to wakeup host while it is in sleep.
        As soon as option 251 is given and as by default GPIO12 on Controller is HIGH, Controller will go to SLEEP.

        Host will use J9 pin 12 to control the controller GPIO18 which comes on Firecrest J10 header pin number 13.

        Host has to make GPIO18 on Controller HIGH to move into DEEPSLEEP mode. As soon as Controller move into DEEPSLEEP mode, Host should not send any HCI command.
        Make sure controller sleep vendor command option 251 is given before this.
        - Enter option 252 in main menu.

        Host has to make GPIO18 on Controller LOW to move into WAKEUP mode so that Controller can accept HCI commands.
        - Enter option 253 in main menu.

        In this case it satisfies wakes up Controller externally using OOB GPIO.

        Controller to Host sleep and wakeup procedure:
        ----------------------------------------------
        Controller will use GPIO19 here to let Host know when to sleep, and when to wakeup.

        Host will use J9 pin 6 to receive interrupt from Controller GPIO19(on FC, J10 header pin number 35).
        Host should sleep as soon as GPIO19 goes HIGH. As soon as Host prints "Host can now go to sleep", enter option 254 in main menu.
        Switch 7 can be used to wakeup host while it is in sleep.

        Host will wakeup as soon as GPIO19 goes LOW (this happens when there is any event pending on host side which controller is sending).
        
        In this case it satisfies Wake on BLE triggered internally in controller wakes up Host externally using OOB GPIO.

8.10 RF test mode steps:
-------------------
This section describes the commands to perform the RF Test for Bluetooth Classic and Bluetooth Low Energy

It is mandated to enable BT_RF_TEST macro and rebuild
Enter all values in HEX format

BT classic menu can be accessed from Main menu->11. HCI menu

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager


     0. Exit.
     1. Refresh

     2. Read Local BD_ADDR.

     3. Inquiry.
     4. Write Scan Enable.

    10. Create Connection (ACL).
    11. Add SCO Connection.
    12. Disconnection.
    13. Remote Name Request.

    15. Setup Synchronous Connection.
    16. Accept/Reject Synchronous Connection Request.
    17. Enhanced Setup Synchronous Connection.
    18. Accept/Reject Enhanced Setup Synchronous Connection Request.
    19. Set Synchronous Connection Request Auto Response.

    20. Get HCI Connection Details.

    22. Read page timeout.
    23. Read connection accept timeout.
    24. Read Class of Device.
    25. Read Local Supported Codecs.
    26. Read Local Supported Features.
    27. Read Local Version Information.
    28. Read Remote Version and Supported Features.
    29. Read inquiry mode.

    30. Role Discovery.
    31. Switch Role.
    32. Sniff Mode.
    33. Park Mode.
    34. Hold Mode.
    35. Exit Sniff Mode.
    36. Exit Park Mode.
    37. Write Link Policy.
    38. Sniff subrating.
    39. QOS Setup.
    40. Write Connection Accept Timeout.
    41. Write Page Timeout.
    42. Write Page Scan Activity.
    43. Write Inquiry Scan Activity.
    44. Write Class of Device.
    45. Write Local Name.
    46. Read Default Link Policy Settings.
    47. Write Default Link Policy Settings.

    50. Write Inquiry Mode.
    51. Write Inquiry Scan Type.
    52. Write Page Scan Type.
    53. Write Flow Control Mode.
    54. Read current IAC LAP.
    55. Write current IAC LAP.
    56. Change ACL/SCO packet type.
    57. Change eSCO packet type.
    58. Cancel inquiry scan

    60. Read Voice Settings.
    61. Read Enhanced Transmit Power Level.
    62. Read Encryption Key Size.
    63. Read Link Policy

    65. Read Secure Connections Host Support.
    66. Write Secure Connections Host Support.
    67. Write Secure Connections Test Mode.
    68. Read HCI Buffer Size.

    70. Write Extended Inquiry Response.
    71. Test Mode - RX Test.
    72. Test Mode - TX Test.

    74. Vendor Specific commands.
    75. HCI generic command.
    76. Enable Test Mode.

    80. LE Operations.

   100. HCI Reset.

Enter option 76 to enable test mode feature
The HCI_Enable_Device_Under_Test_Mode command is used to enter test mode via LMP test commands for BR/EDR controllers.

Enter option 72 to start transmit test
This command sets the transmit test parameters. An HCI reset command is required after this test to resume normal Bluetooth operation

Code will fill by default RxOnStart, SyntOnStart, TxOnStart, PhdOffStart with 0x80

Test scenario
0x01 = PATTERN_00 (data pattern: 0x00)
0x02 = PATTERN_FF (data pattern: 0xFF)
0x03 = PATTERN_55 (data pattern: 0x55)
0x04 = PATTERN_PRBS (data pattern: 0xFE)
0x09 = PATTERN_0F (data pattern: 0x0F)
0xFF = exit test

Hopping mode:
0x00 = fix frequency
0x01 = hopping set

Transmit/Receive Frequency = (2402+k) MHz, where k is the value of Tx/Rx Channel

Poll interval in frames for the link (units, 1.25 ms)

Transmit Packet Type
0x03 = DM1
0x04 = DH1
0x0A = DM3
0x0B = DH3
0x0E = DM5
0x0F = DH5
0x14 = 2-DH1
0x18 = 3-DH1
0x1A = 2-DH3
0x1B = 3-DH3
0x1E = 2-DH5
0x1F = 3-DH5

Whitening
0x00 = disabled
0x01 = enabled

Number of test packets 0 = infinite (default)

Tx power
Signed value of Tx power (dBm)
Range = -20 dBm to 12 dBm (default = 4 dBm)

Example:
Enter Test Scenario :
1

Enter Hopping Mode :
0

Enter Tx Channel :
0

Enter Rx Channel :
0

Enter Tx Test Interval :
1

Enter Packet Type :
F

Enter Length Of Test Data :
25

Enter Whitening :
0

Enter Number Of Packets :
0

Enter Tx Power :
4

Sent Command TX_TEST

Enter option 71 to start receive test
This command sets the receive test parameters. An HCI reset command is required after this test to resume normal Bluetooth operation

Test scenario
0x01 = receiver test, 0- pattern
0x02 = receiver test, 1- pattern
0x03 = receiver test, 1010- pattern
0x04 = receiver test, PRBS- pattern
0x09 = receiver test, 1111 0000- pattern
0xFF = abort test mode

Transmit/ receive frequency
f = (2402+k)MHz

Test Packet Type
0x03 = DM1
0x04 = DH1
0x0A = DM3
0x0B = DH3
0x0E = DM5
0x0F = DH5
0x14 = 2-DH1
0x18 = 3-DH1
0x1A = 2-DH3
0x1B = 3-DH3
0x1E = 2-DH5
0x1F = 3-DH5

Length of Test Data: Should not be bigger than the maximum size of the specified test packet type

Report Error Packets
0x00 = none (default)
0x01 to 0xFE = number of packets to report

Example:
Enter Test Scenario :
4

Enter Tx Frequency :
0

Enter Rx Frequency:
0

Enter Packet Type :
F

Packet Type : F
Enter Number Of Packets :
5DC

num_packets= 5DC
Enter Length Of Test Data :
25

Length OF Test Data= 25
Enter Tx AM Address :
1

Enter Tx BD Address : (have to set tx address)
EE FF C0 88 00 00

Enter Report Error Packets :
0

Sent Command RX_TEST

To stop tests, in test scenario give FF

BT LE menu can be accessed from Main menu->11. HCI menu->80. LE operations

To start a test where the DUT generates test reference packets at a fixed interval, use LE Transmitter Test[V2] command.
51. Transmitter Test Command.
TX_Channel
N = (F-2402) / 2
Range: 0x00 to 0x27
Frequency Range: 2402 MHz to 2480 MHz

Test_Data_Length
0x00 to 0xFF Length in bytes of payload data in each packet

Packet_Payload
0x00 PRBS9 sequence '11111111100000111101' (in transmission order) as
0x01 Repeated '11110000' (in transmission order)
0x02 Repeated '10101010' (in transmission order)
0x03 PRBS15 sequence
0x04 Repeated '11111111' (in transmission order) sequence
0x05 Repeated '00000000' (in transmission order) sequence
0x06 Repeated '00001111' (in transmission order) sequence
0x07 Repeated '01010101' (in transmission order) sequence

PHY
0x01 Transmitter set to use the LE 1M PHY
0x02 Transmitter set to use the LE 2M PHY
0x03 Transmitter set to use the LE Coded PHY with S=8 data coding
0x04 Transmitter set to use the LE Coded PHY with S=2 data coding

To start a test where the DUT receives test reference packets at a fixed interval, use LE Receiver Test[V2] command
50. Receiver Test Command.
RX_Channel,
N = (F-2402) / 2
Range: 0x00 to 0x27.
Frequency Range: 2402 MHz to 2480 MHz

PHY,
0x01 Receiver set to use the LE 1M PHY
0x02 Receiver set to use the LE 2M PHY
0x03 Receiver set to use the LE Coded PHY

Modulation_Index
0x00 Assume transmitter will have a standard modulation index
0x01 Assume transmitter will have a stable modulation index

To stop any test which is in progress, use LE Test End command.
52. End Test Command.


        8.11 BLE audio GA profiles

        Steps to setup BAP Streaming:
        ----------------------------
#####################################
Legacy options
2.  EtherMind Init.
           3.  Bluetooth ON.
           200. GA Profile Option.
           2.  GA Init.
        
        B) Initialization of ASCS and PACS on Server.
        BAP Server:
           3.  GA Register - PACS.
               [PACS]GA capability ID: 0x01
               [PACS]Registering GA capability: Retval: 0x0000
               Registered ID : 0x01
               [PACS]Registering GA Audio Contexts and Locations: 0x0000

           4.  [ASCS]Registering ASE
           [ASCS]Registered ASE ID, Retval - 0x0000
           Sink ASE ID 1
        
            50. LE Adv. Enable.
            <<Advertise the device for connection>>
        
             0. Return to main menu
        
           210. GA Setup.
                <<Dynamic DB Setup>>
        
        
        
        C) BLE Extended Connection Initiate and SMP Authenticate, and MTU Exchange.
        BAP Client:
           0
           210. GA Setup.
                <<Dynamic DB Setup>>
           60. Set peer endpoint
           Enter endpoint address: xx xx xx xx xx xx <Enter BAP Server Address>
           Enter endpoint type: 0
           52. LE Extended Connection
        
           Verify below and take note of "Connection Handle"
Received HCI_LE_META_EVENT.
        Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
        Connection Handle: 0x0000
        Channel Selection Algorithm: 0x01
        
           55. LE Authenticate and Encrypt
        
        Verify:
        Recvd SMP_AUTHENTICATION_COMPLETE
        Status : 0000
        
        BAP Server:
          NA
        
        D) BAP Procedures
        BAP Client:
           20. Setup PACS - Audio Role Discovery.
        
        Verify:
        Recvd GA_SETUP_CNF
                Roles Supported - 0x03
                [Indicates the presence of both Src and Sink ASE]
        
           22. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts
           Enter Role (1 - Sink, 2 - Source):
           1
        
        Verify:
        Recvd GA_GET_CAPABILITIES_CNF
        
           22. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts
           Enter Role (1 - Sink, 2 - Source):
           2
        
        Verify:
        Recvd GA_GET_CAPABILITIES_CNF
        
           25. ASE Discover - ASE_ID Discovery.
        
        Verify:
           Recvd GA_SEP_DISCOVER_CNF
        
           Codec configuration of ASEs
           26. ASE Codec Configuration.
           Enter ASE Count: 1
             Enter ASE ID: 1
             Enter ASE Role: 1
             Enter the Capability Set Index: 0 [Refer "GA_GET_CAPABILITIES_CNF" for index]
        
        Verify:
           Recvd GA_SEP_CONFIGURE_CNF with Status 0x0000
        
           27. ASE Setup CIG Parameters.
           Enter CIG ID: 1
           Enter CIS Count: 1
             Enter ASE ID: 1
             Enter CIS ID: 1
        
        Verify:
           Recvd HCI_COMMAND_COMPLETE_EVENT
           CIG ID: 1
           CIS Count: 1
           CIS Conn Handle: 0x0002 <<Note the handle here>>
        
           QoS configuration of ASEs
           28. ASE QoS Configuration.
           Enter ASE Count: 1
             Enter ASE ID: 1
        
        Verify:
           Recvd GA_SEP_SETUP_CNF with Status 0x0000
           Start the ASEs
           29. ASE Enable.
           Enter ASE Count: 1
             Enter ASE ID: 1
             Enter Context Type: 4
             Enter CCID Count: 1
               Enter CCID: 1
        
        Verify:
           Recvd GA_SEP_START_CNF with Status 0x0000
        
           Establish the CIS
           30. ASE Setup CIS.
           Enter CIS Count: 1
             Enter ASE ID: 1
        
        Verify:
           HCI_LE_CIS_ESTABLISHED_SUBEVENT. with status as 0x00
        
        BAP Server:
        Verify:
           HCI_LE_CIS_REQUEST_SUBEVENT.
        
        Verify:
           HCI_LE_CIS_ESTABLISHED_SUBEVENT. with status as 0x00
        
        << Streaming should begin. Ensure the "samplewav" GA resource is present in the FS root directory of binary access>>
        
           To update streaming Metadata:
           32. ASE Update.
           Enter ASE Count: 1
             Enter ASE ID: 1
             Enter Contexts: 4
             Enter CCID Count: 1
               Enter CCID: 1
        
        Verify:
           Recvd GA_SEP_UPDATE_CNF with Status 0x0000
        
           To Suspend streaming:
           33. ASE Disable.
           Enter ASE Count: 1
             Enter ASE ID: 1
        
        Verify:
           Recvd GA_SEP_SUSPEND_CNF with Status 0x0000
        
           To resume streaming:
        BAP Client:
           29. ASE Enable
           Enter ASE Count: 1
             Enter ASE ID: 1
             Enter Context Type: 4
             Enter CCID Count: 1
               Enter CCID: 1
        
        BAP Server:
           7.  ASE Send Autonomous Rx Start Ready for Local ASE Sink.
              Enter endpoint address: xx xx xx xx xx xx <Enter BAP Client Address>
              Enter endpoint type: 0
              Enter ASE ID: 1
        
        Verify:
           Recvd GA_SEP_START_CNF with Status 0x0000
        
           To Release streaming:
           35. ASE Release.
           Enter ASE Count: 1
             Enter ASE ID: 1
        
        Verify:
           Recvd GA_SEP_RELEASE_CNF with Status 0x0000
        
           To Save files and close dumps:
           15. Save Audio Dump to Files.
        
        BAP Server:
           To Save files and close dumps:
           15. Save Audio Dump to Files.


        Steps to setup GA Broadcast Streaming:
        -------------------------------------
        
        Initialization of Stack and BAP on Broadcast Soruce and Sink:
           2.  EtherMind Init.
           3.  Bluetooth ON.
           200. GA Profile Option.
           2.  GA Init. 
        
        Broadcast Source:   
        A) Allocate and Configure Broadcast session on source.
           80. Allocate Session
           82. Configure Session
        
        B) Register Source Endpoint with CodecInfo, Metadata, NumStreams etc to configured session
           83. Register Source Endpoint
        
        C) Advertise Announcements (Broadcast and Basic)
           84. Setup Announcement
        
        Verify:
           GA_BC_SETUP_ANNOUNCEMENT_CNF with Status 0x0000
        
        D) Start Broadcast Streaming
           86. Start Broadcast
        
        Verify:
           GA_BC_START_CNF with Status 0x0000
        
        E) Stop Broadcast Streaming and Stop Advertising Announcements
           87. Stop Broadcast
           85. End Announcement
        
        Verify:
           GA_BC_SUSPEND_CNF with Status 0x0000
           GA_BC_END_ANNOUNCEMENT_CNF with Status 0x0000
           
        Broadcast Sink:
        A) Scan for available broadcast announcements
           90. Scan Announcement
        
        Verify:
           GA_BC_SCAN_ANNOUNCEMENT_CNF with Status 0x0000
           
        Verify:
           GA_BC_SOURCE_ANNOUNCEMENT_IND
        
        B) Synchronize to the Advertisements in the Announcements
           92. Associate Stream
               Enter SID: << Enter SID of the desired announcement displayed >>
               Enter endpoint address: << Enter the BD Address of the announcement source >>
               Enter endpoint type: << Enter the BD Address Type of the announcement source >>
        
        Verify:
           GA_BC_ASSOCIATE_CNF with Status 0x0000
        
        Verify:
           GA_BC_SOURCE_CONFIG_IND
        
        Verify:
           GA_BC_SOURCE_STREAM_IND
        
        C) Synchronize to the Broadcast stream and Start streaming
           94. Enable Stream
        
        Verify:
           GA_BC_ENABLE_CNF with Status 0x0000
        
        D) Disable Stream and Dissociate from Source
           95. Disable Stream
           93. Dissociate Stream
        
        Verify:
           GA_BC_DISABLE_CNF with Status 0x0000
        
        Verify:
           GA_BC_DISSOCIATE_CNF with Status 0x0000
#####################################
        
#####################################
New updated options
#####################################
        
        
        Steps to for CCP(Call Control Profile):
        -----------------------------------------
        A) Initialization of BAP, CAP and GA Bearer Layer.
        BAP Client and BAP Server:
           2.  EtherMind Init.
           3.  Bluetooth ON.
           200. GA Profile Option.
           2.  GA Init.
        
        B) Extended Connection Initiate and SMP Authenticate, MTU Exchange.
           60. Set peer endpoint
           Enter endpoint address: xx xx xx xx xx xx
           Enter endpoint type: 0
           52. LE Extended Connection
        
           Verify below and take note of "Connection Handle"
Received HCI_LE_META_EVENT.
           Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
           Connection Handle: 0x0000
           Channel Selection Algorithm: 0x01
        
           55. LE Authenticate and Encrypt
        
           GATT Client Operations
           90. Exchange MTU - 64
        
        <<Now, BLE Connection & Authentication is done, ready to perform CCP operations>>
        
        C) For CCP Server Setup:
           202. GA CCP Server Operations
           2 - CCP Server Init        <- To Initialize CCP Server Module
           4 - CCP Register TBS Entity <- Register TBS Service
           7 - Set Peer BD Address <- Register Peer Address
        
        D) For CCP Client Setup:
           201. GA CCP Client Operations.
            2 - CCP Init
            4 - CCP Setup GTBS Context
                Enter Peer BD Address: xx xx xx xx xx xx
                Enter endpoint type: x
            <Wait for 'CCP_CE_SETUP_CNF' Event>
            5 - CCP Discover TBS(s) with peer device
                Enter Peer BD Address: xx xx xx xx xx xx
                Enter endpoint type: x
               <Wait for 'CCP_CE_DISCOVER_TBS_CNF' Event>
               GA Status: 0x00C0 <- Continue response.
               <Note down the Start & End Handles of TBS Service>
               GA Status: 0x0000 <- Final response.
        
            6 - CCP Setup TBS Context
                <Enter correct handles of TBS>
               Enter start handle(in Hex):
               Enter end handle(in Hex):
               <Wait for 'CCP_CE_SETUP_TBS_CNF' Event>
        
        E) For CCP Controller Operations:
           - Select appropriate menu options and wait for confirm event.
        
           Example:
           1. 10 - CCP Read Bearer Provider Name
              Wait for 'CCP_CE_RD_BRR_PROVIDER_NAME_CNF' Event.
        
           2. 11 - CCP Read Bearer UCI
              Wait for 'CCP_CE_RD_BRR_UCI_CNF' Event.
        
           3. 12 - CCP Read Bearer Technology
              Wait for 'CCP_CE_RD_BRR_TECH_CNF' Event.
        
        F) For CCP Server Operations:
           - Select appropriate menu options for Sending the Notification,
           Example:
            11 - Send Char Notify
            10 -> CCP Notify Bearer Provider Name
        
        
        Steps to for MCP(Media Control Profile):
        -----------------------------------------
        A) Initialization of BAP, CAP and GA Bearer Layer.
        BAP Client and BAP Server:
           2.  EtherMind Init.
           3.  Bluetooth ON.
           200. GA Profile Option.
           2.  GA Init.
        
        B) Extended Connection Initiate and SMP Authenticate, MTU Exchange.
           60. Set peer endpoint
           Enter endpoint address: xx xx xx xx xx xx
           Enter endpoint type: 0
           52. LE Extended Connection
        
           Verify below and take note of "Connection Handle"
Received HCI_LE_META_EVENT.
           Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
           Connection Handle: 0x0000
           Channel Selection Algorithm: 0x01
        
           55. LE Authenticate and Encrypt
        
           GATT Client Operations
           90. Exchange MTU - 64
        
        <<Now, BLE Connection & Authentication is done, ready to perform MCP operations>>
        
        C) For MCP Server Setup:
           204. GA MCP Server Operations.
           5.  GA MCS Initialize   <- To Initialize MCP Server Module
           11. Add MCS Instance: <- To register MCS service
        
        D) For MCP Client Setup:
           204. GA MCP Server Operations.
            5.  GA MCP CE Init
            11. Setup GMCS session with Peer Device
                Enter Peer BD Address: xx xx xx xx xx xx
                Enter endpoint type: x
            <Wait for Context Setup Complete: GMCS Instance: 0x00>
            12. Discover MCS instances on remote device
                Enter Peer BD Address: xx xx xx xx xx xx
                Enter endpoint type: x
            <Wait for MCS Service Discovery Result: Success>
               GA Status: 0x00C0 <- Continue response.
               <Note down the Start & End Handles of MCS Service>
               GA Status: 0x0000 <- Final response.
        
            13. Setup MCS session with Peer Device
                <Enter correct handles of MCS>
               Enter start handle(in Hex):
               Enter end handle(in Hex):
               <Wait for Context Setup Complete: MCS Instance: 0x00>
        
        E) For MCP Controller Operations:
           - Select appropriate menu options and wait for confirm event.
        
           Example:
           1. 20. Read Media Player Name
              Wait for 'MCP_CE_READ_MEDIA_PLAYER_NAME_CNF' Event.
        
           2. 25. Read Track Title
              Wait for 'MCP_CE_READ_TRACK_TITLE' Event.
        
        F) For MCP Server Operations:
           - Select appropriate menu options for Sending the Notification,
           Example:
            13. Notify Media Player Name
            14. Notify Track Title
        
        
        Steps to for VCP(Volume Control Profile):
        -----------------------------------------
        A) Initialization of BAP, CAP and GA Bearer Layer.
        BAP Client and BAP Server:
           2.  EtherMind Init.
           3.  Bluetooth ON.
           200. GA Profile Option.
           2.  GA Init.
        
        B) Extended Connection Initiate and SMP Authenticate, MTU Exchange.
           60. Set peer endpoint
           Enter endpoint address: xx xx xx xx xx xx
           Enter endpoint type: 0
           52. LE Extended Connection
        
           Verify below and take note of "Connection Handle"
Received HCI_LE_META_EVENT.
           Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
           Connection Handle: 0x0000
           Channel Selection Algorithm: 0x01
        
           55. LE Authenticate and Encrypt
        
           GATT Client Operations
           90. Exchange MTU - 64
        
        <<Now, BLE Connection & Authentication is done, ready to perform VCP operations>>
        
        C) For VCP Renderer Setup:
           41. VCP Renderer Operations.
           7. Register Peer Device Address.
        
        D) For VCP Controller Setup:
           40. VCP Controller Operations.
            2. VCP Setup
               <Enter Peer Address>
              Wait for GA_VC_SETUP_CNF
        
            3. VCP Get Capabilities <= To discover included services
               Wait for GA_VC_GET_CAPABILITIES_CNF event.
               GA Status: 0xC0 <- Continue response.
               GA Status: 0x00 <- Final response.
               <Note down the Start & End Handles of VOCS/AICS services>
        
            4. VCP Setup VOCS Capability <- To setup the VOCS
               - Enter Valid Start/End Handles of the VOCS service
        
            5. VCP Setup AICS Capability <- To setup the AICS
               - Enter Valid Start/End Handles of the AICS service
        
        E) For VCP Controller operation:
           - Select appropriate menu options and wait for confirm event.
        
           Example:
           1. 10. VCS Get Volume State
              Wait for 'GA_VC_GET_VOLUME_STATE_CNF' Event.
        
           2. 12. VCS Send Relative Volume Down
              Wait for 'GA_VC_VOLUME_STATE_NTF' Event.
        
        F) For VCP Renderer operations:
           - Select appropriate menu options for sending the notification.
        
        G) To release the service
           8. VCP release AICS
           7. VCP release VOCS
           6. VCP release VCS


########################################
TMAP client

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?2
choice is 2
Initializing EtherMind ...
EtherMind Stack Successfully Initialized!

   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?3
choice is 3
Performing Bluetooth ON ...

Default GAP Role Set to PERIPHERAL
Initialize all required GA modules and then register the database.


   0.  Exit.
   1.  Refresh this Menu.

   2.  EtherMind Init.
   3.  Bluetooth ON.
   4.  Bluetooth OFF.

   5.  Set PIN Code.
   6.  Set Device Role.

   7.  Get Free ACL Tx Queue Buffer.
   8.  Set LWM/HWM for ACL Tx Queue.

   9.  Configure Service GAP Procedures
   10. Set Snoop logging.

   11. HCI Operations.
   12. SDP Operations.
   13. SDDB Operations.
   14. RFCOMM Operations.
   15. MCAP Operations.
   16. BNEP Operations.
   17. AVDTP Operations.
   18. AVCTP Operations.
   19. OBEX Client Operations.
   20. OBEX Server Operations.
   21. L2CAP Operations.

   25. Write Storage event mask.

   30. Security Manager.
   31. LE SMP.
   32. LE L2CAP.
   33. ATT.
   34. L2CAP ECBFC.

   40. SPP Operations.
   41. HFP Unit Operations.
   42. HFP AG Operations.
   45. DUNP DT Operations.
   46. DUNP GW Operations.
   47. SAP Client Operations.
   48. SAP Server Operations.

   50. OPP Client Operations.
   51. OPP Server Operations.
   52. FTP Client Operations.
   53. FTP Server Operations.
   54. MAP Client Operations.
   55. MAP Server Operations.
   56. PBAP Client Operations.
   57. PBAP Server Operations.
   58. CTN Client Operations.
   59. CTN Server Operations.
   60. BIP Initiator Operations.
   61. BIP Responder Operations.
   62. SYNCP Client Operations.
   63. SYNCP Server Operations.

   65. A2DP Operations.
   66. AVRCP Operations.
   67. HDP Operations.
   68. PAN Operations.

   70. HID Device Operations.
   71. HID Host Operations.

   75. DID Client Operations.
   76. DID Server Operations.

   80. GATT Client Operations.
   81. GATT Server Operations.

   90. BPP Sender Operations.
   91. BPP Printer Operations.

  100. Simulate VU.

  200. GA Legacy Options.

  201. GA CCP Client Operations.
  202. GA CCP Server Operations.

  203. GA MCP Client Operations.
  204. GA MCP Server Operations.

  205. GA TMAP Operations.
  206. GA HAP Operations.
  207. GA BASS Operations.

  210. GA Setup.

  220. GA Profile Options.

  250. Wake on BLE vendor command.
  251. H2C sleep vendor command.
  252. H2C sleep.
  253. H2C wakeup.
  254. Host sleep.

  280. Disable Logging.
  281. Enable Logging.

  300. BEACON Manager

 Your Option ?Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C03 -> HCI_RESET_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C03 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C01 -> HCI_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C01 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C63 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C63 -> UNKNOWN
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2001 -> HCI_LE_SET_EVENT_MASK_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C56 -> HCI_WRITE_SIMPLE_PAIRING_MODE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C56 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1009 -> HCI_READ_BD_ADDR_OPCODE
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
31 12 63 E8 07 C0                                  1.c...
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1009 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
31 12 63 E8 07 C0                                  1.c...
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2060 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2060 -> UNKNOWN
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 01 04 00 01 05                                  ......
-------------------------------------------------------------------


Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x1005 -> HCI_READ_BUFFER_SIZE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x1005 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C13 -> HCI_CHANGE_LOCAL_NAME_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C13 -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C18 -> HCI_WRITE_PAGE_TIMEOUT_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C18 -> UNKNOWN
        LE Command Status: 0x01

>       Bluetooth ON Initialization Completed.
>       Bluetooth Address: C0:07:E8:63:12:31

>       Stack Version - 020.006.000.
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x080F -> HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x080F -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C1A -> HCI_WRITE_SCAN_ENABLE_OPCODE
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C1A -> UNKNOWN
        LE Command Status: 0x01
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x0C6D -> UNKNOWN
        Command Status: 0x01

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x0C6D -> UNKNOWN
        LE Command Status: 0x01
220
choice is 220


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 2
Registering HCI Callback Function at Location 0
Number of Registered HCI Callback Functions = 1
HCI Callback Registered
Configuring GA
Initializing GA
Registering HCI Callback Function at Location 1
Number of Registered HCI Callback Functions = 2
Retval - GA_SUCCESS (0x0000)
Default settings done for device manager
GA_vc_register. for VOCS
VCP_VOCS Handle: 0x00
Retval - 0x0000
GA_vc_register..for AICS
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2074 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2074 -> HCI_LE_SET_HOST_FEATURE_OPCODE
        LE Command Status: 0x00
VCP_AICS Handle: 0x00
Retval - 0x0000
GA_mc_register_aics
AICS Handle: 0x00
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 2
GA_tmap_init...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
0
 - Call Gateway(CG): 0x0001
 - Call Terminal(CT): 0x0002
 - Unicast Media Sender (UMS): 0x0004
 - Unicast Media Receiver (UMR): 0x0008
 - Broadcast Media Sender (BMS): 0x0010
 - Broadcast Media Receiver (BMR): 0x0020
Note: If this device supports more than one role, Compute BitMask OR and input
E.g., Call Gateway(CG):0x0001 and Call Terminal(CT):0x0002, Input 0x0003
4
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 3
Input TMAP Type, Client: 1, Server: 0
1
GA_tmap_register_role...
Retval - GA_SUCCESS (0x0000)

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 0


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 23
1. Enable 2. Disable
1


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2041 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2041 -> HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_OPCODE
        LE Command Status: 0x00
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2042 -> UNKNOWN
        Command Status: 0x00

Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2042 -> HCI_LE_SET_EXTENDED_SCAN_ENABLE_OPCODE
        LE Command Status: 0x00
HCI_LE_META_EVENT
HCI_LE_EXTENDED_ADVERTISING_REPORT_SUBEVENT
        - num_reports = 0x01
        - Event type: 0x0001
        - Address type: 0x0000
        - Address: 0x16 0x9C 0x01 0xE8 0x07 0xC0
        - Primary PHY: 0x0001
        - Secondary PHY: 0x0001
        - Advertising SID: 0x0001
        - Tx Power: 0x007F
        - RSSI: 0x00EA
        - Periodic Advertising Interval: 0x0000
        - Direct Address type: 0x0000
        - Direct Address:0x00 0x00 0x00 0x00 0x00 0x00
        - Data Length: 0x001E
        - Data:
                - AD Length: 0x0002
                - AD Type: 0x0001
                - AD Value:
-- Dumping 1 Bytes --
-------------------------------------------------------------------
02                                                 .
-------------------------------------------------------------------



                - AD Length: 0x0004
                - AD Type: 0x0016
                - AD Value: Service UUID: Common Audio Service (0x1853)

-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------



                - AD Length: 0x0004
                - AD Type: 0x0016
                - AD Value: Service UUID: Audio Stream Control Service (0x184E)

-- Dumping 1 Bytes --
-------------------------------------------------------------------
00                                                 .
-------------------------------------------------------------------



                - AD Length: 0x0003
                - AD Type: 0x0016
                - AD Value: Service UUID: Broadcast Audio Scan Service (0x184F)


                - AD Length: 0x0005
                - AD Type: 0x0003
                - AD Value:
-- Dumping 4 Bytes --
-------------------------------------------------------------------
4F 18 50 18                                        O.P.
-------------------------------------------------------------------



                - AD Length: 0x0006
                - AD Type: 0x0008
                - AD Value:
-- Dumping 5 Bytes --
-------------------------------------------------------------------
4D 74 2D 47 41                                     Mt-GA
-------------------------------------------------------------------



26
Enter endpoint address:  16 9C 01 E8 07 C0
BD_ADDR Entered 169C01E807C0
Enter endpoint type: 0
Initiating LE Extended Connection...
API returned success...


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2043 (UNKNOWN)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2043 (HCI_LE_EXTENDED_CREATE_CONNECTION_OPCODE)
[ATT]:[0x0D: 0x05]: Received ATT Event 0x81 with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
16 9C 01 E8 07 C0 00                               .......
-------------------------------------------------------------------

Received ATT Connection Indication, Result 0x0000!
[ATT] ATT connected over LE Fixed Channel!
[ATT HANDLE]: (Dev ID: 0x0D, ATT ID: 0x05

[IPSPR]: ACL Connected with IPSP Node

[CPMC]: ACL Connected with CPM Sensor
[ATT]:[0x0D]:[0x05]: Received event 0x81 with result 0x0000
Received ATT Connection Indication with result 0x0000

Received ATT_CONNECTION_IND on LE for ATT Handle:
   -> Device_ID       : 0x0D
   -> ATT_Instance_ID : 0x05

[BASIC]: In appl_basic_connect
Subevent : HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVENT.
status = 0x00
connection_handle = 0x0000
role = 0x00
appl_peer_addr_type = 0x00
appl_peer_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
16 9C 01 E8 07 C0                                  ......
-------------------------------------------------------------------

local_resolvable_pvt_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 00 00 00 00 00                                  ......
-------------------------------------------------------------------

peer_resolvable_pvt_addr =

-- Dumping 6 Bytes --
-------------------------------------------------------------------
00 00 00 00 00 00                                  ......
-------------------------------------------------------------------

conn_interval = 0x0050
conn_latency = 0x0000
supervision_timeout = 0x09D0
clock_accuracy = 0x07
Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
Connection Handle: 0x0000
Channel Selection Algorithm: 0x01
[ATT]:[0x0D: 0x05]: Received ATT Event 0x02 with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
40 00                                              @.
-------------------------------------------------------------------

Received Exchange MTU Request with Result 0x0000. MTU Size = 0x0040!
[ATT]:[0x0D]:[0x05]: Received event 0x02 with result 0x0000
Received Exchange MTU Request with MTU Size = 0x0040!
Sent Response with retval 0x0000

[BASIC]: Updated MTU is 64 for Appl Handle 0x05
[ATT]:[0x0D: 0x05]: Received ATT Event 0x03 with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
40 00                                              @.
-------------------------------------------------------------------

Received Exchange MTU Response with Result 0x0000. MTU Size = 0x0040!
28
Device:
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
Connection Handle: 0x0000
Initiator: 0x01
Index: 0x01

Enter the Conn device index: 1
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2019 (HCI_LE_START_ENCRYPTION_OPCODE)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2019 (HCI_LE_START_ENCRYPTION_OPCODE)

Recvd SMP_AUTHENTICATION_COMPLETE
BD Address : 16 9C 01 E8 07 C0
BD addr type : Public Address
Status : 0000

Confirmed Authentication using Encryption
[BASIC]: In appl_basic_connect
Received HCI_ENCRYPTION_CHANGE_EVENT.
        Status: 0x00
 -> HCI_Encryption Status Succeeded
        Connection Handle: 0x0000
        Encryption Enable: 0x01 -> Encryption ON

LE: Unknown Event Code 0x08 Received.


1


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 3
GA Setup - Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 10
Device:
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
Connection Handle: 0x0000
Initiator: 0x01
Index: 0x01

Enter the Conn device index: 1
GA_tmap_discover...
Retval - GA_SUCCESS (0x0000)
appl_tmap_handle:0x0000

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? [ATT]:[0x0D: 0x05]: Received ATT Event 0xF0 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
55                                                 U
-------------------------------------------------------------------

Received GATT_PS_DISCOVERY_RSP
No. Primary Services - 1

UUID: 0x1855 (Unknown)
Start Hdl: 0x0070, End Hdl: 0x0072

[ATT]:[0x0D: 0x05]: Received ATT Event 0xF3 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
71                                                 q
-------------------------------------------------------------------

Received GATT_CHAR_DISCOVERY_RSP
No. Characteristics - 1

(Unknown)
Char Handle: 0x0071, UUID: 0x2B51
Property: 0x02, Value Handle: 0x0072

[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
08 00                                              ..
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 2 Bytes --
-------------------------------------------------------------------
08 00                                              ..
-------------------------------------------------------------------

************************** CALLBACK: GA TMAP Client **************************
[Profile]     : TMAP (0x1855)
[SECTION]     : TMAP CLIENT EVENTS
[SUB-SECTION] : TMAP CLIENT-DISCOVER
[TYPE]        : DISCOVER RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : TMAP_DISCOVER_CNF (0x01)
[Service Cntx]: 0xCC

-----------------------------------------------------------------
Data Length: 0x02
Data:
Len: 0x02,  Role(s): 0x0008
        -Unicast Media Receiver (UMR) (0x0008)
*********************************************************************
[ATT]:[0x0D: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
01 00 FF FF 00 28 55 18                            .....(U.
-------------------------------------------------------------------

[ATT]:[0x0D]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0001-0xFFFF
[APPL]: Number of occurrences of UUID 0x0000 = 0x0001
[APPL]: <<< Handling Find By Type Value request, Range 0x0001-0xFFFF, with result 0x0000
[ATT]:[0x0D: 0x05]: Received ATT Event 0x06 with result 0x0000

-- Dumping 8 Bytes --
-------------------------------------------------------------------
73 00 FF FF 00 28 55 18                            s....(U.
-------------------------------------------------------------------

[ATT]:[0x0D]:[0x05]: Received event 0x06 with result 0x0000
Received Find By Type Value Request with result 0x0000
[APPL]: >>> Handling Find By Type Value request, Range 0x0073-0xFFFF
[APPL]: Sent Error Response with result 0x0000
[APPL]: <<< Handling Find By Type Value request, Range 0x0073-0xFFFF, with result 0x0000
[ATT]:[0x0D: 0x05]: Received ATT Event 0x08 with result 0x0000

-- Dumping 6 Bytes --
-------------------------------------------------------------------
70 00 72 00 03 28                                  p.r..(
-------------------------------------------------------------------

[ATT]:[0x0D]:[0x05]: Received event 0x08 with result 0x0000
[APPL]: >>> Handling Read By Type request, Range 0x0070-0x0072
Received Requests from Peer on the Server Side with Op:0xF5 from DEVID:0x0D Handle:0x0071 SID:0x0B CID:0x0027
[APPL]:[0x0000]: Handle 0x0071 -> Value Len 0x0005
[APPL]:Search Complete, #Handle UUID List found = 0x01
[APPL]: Sent Read By Type Response with result 0x0000
[APPL]: <<< Handling Read By Type request, Range 0x0070-0x0072
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0A with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
72 00                                              r.
-------------------------------------------------------------------

[ATT]:[0x0D]:[0x05]: Received event 0x0A with result 0x0000
Received Requests from Peer on the Server Side with Op:0x81 from DEVID:0x0D Handle:0x0072 SID:0x0B CID:0x0027
1

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 75
Invalid Choice

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 0


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 10
Setup Endpoint
Setup PACS: Audio Role Discovery - PACS Discovery
Retval - GA_SUCCESS (0x0000)
Context: 0


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0xF0 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
50                                                 P
-------------------------------------------------------------------

Received GATT_PS_DISCOVERY_RSP
No. Primary Services - 1

UUID: 0x1850 (Published Audio Capability Service)
Start Hdl: 0x0073, End Hdl: 0x007F

[ATT]:[0x0D: 0x05]: Received ATT Event 0xF3 with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
74 00 00 00                                        t...
-------------------------------------------------------------------

Received GATT_CHAR_DISCOVERY_RSP
No. Characteristics - 4

(Unknown)
Char Handle: 0x0074, UUID: 0x2BCE
Property: 0x12, Value Handle: 0x0075
No. Characteristic Descriptors: 1
Desc Handle: 0x0076, Desc UUID: 0x2902 (CCCD)

(Unknown)
Char Handle: 0x0077, UUID: 0x2BCD
Property: 0x12, Value Handle: 0x0078
No. Characteristic Descriptors: 1
Desc Handle: 0x0079, Desc UUID: 0x2902 (CCCD)

(Sink PAC)
Char Handle: 0x007A, UUID: 0x2BC9
Property: 0x12, Value Handle: 0x007B
No. Characteristic Descriptors: 1
Desc Handle: 0x007C, Desc UUID: 0x2902 (CCCD)

(Sink Audio Locations)
Char Handle: 0x007D, UUID: 0x2BCA
Property: 0x1A, Value Handle: 0x007E
No. Characteristic Descriptors: 1
Desc Handle: 0x007F, Desc UUID: 0x2902 (CCCD)

[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
************************** CALLBACK: GA Client **************************
[Profile]     : PACS (0x1850)
[SECTION]     : PACS CLIENT EVENTS
[SUB-SECTION] : PACS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SETUP_CNF (0x01)

-----------------------------------------------------------------
Data Length: 0x01
Data:
Len: 0x01,  Supported Role: Sink(0x01)
*********************************************************************
12
Enter Role: 1 - Sink, 2 - Source:
1
Get PACS Capabilities - Read Audio Capability, Location, Supp/Avail Contexts
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 31 Bytes --
-------------------------------------------------------------------
01 06 00 00 00 00 13 03 01 80 00 02 02 02 02 03    ................
01 05 04 00 00 64 00 02 05 01 04 03 01 00 00       .....d.........
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 31 Bytes --
-------------------------------------------------------------------
01 06 00 00 00 00 13 03 01 80 00 02 02 02 02 03    ................
01 05 04 00 00 64 00 02 05 01 04 03 01 00 00       .....d.........
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : PACS (0x1850)
[SECTION]     : PACS CLIENT EVENTS
[SUB-SECTION] : PACS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_CONTINUE (0x00C0)
[Event]       : GA_GET_CAPABILITIES_CNF (0x03)

-----------------------------------------------------------------
Char UUID: Sink PAC (0x2BC9)
-----------------------------------------------------------------
Role: Sink(0x01)
Capability Type: Codecs (0x01)
Data Length: 0x1F
Data:
Len: 0x01,  Num of PAC Records: 0x01
Len: 0x05,  Codec ID:
                - Len: 0x01,  Coding Format: LC3 (0x06),
                - Len: 0x02,  Company ID: (0x0000)
                - Len: 0x02,  Vendor Specific Codec ID: (0x0000)
Len: 0x01,  Codec Specific Capabilities Length: 0x13
Len: 0x13,  Codec Specific Capabilities:
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x01
                - Len: 0x02,  Value: Supported Sampling Frequencies: 0x0080
                                48000 Hz

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x02
                - Len: 0x01,  Value: Supported Frame Durations: 0x02
                                10 ms

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x03
                - Len: 0x01,  Value: Supported Audio Channel Counts: 1 (0x01)

                - Len: 0x01,  Length: 0x05
                - Len: 0x01,  Type: 0x04
                - Len: 0x04,  Value: Supported Octets per Codec Frame: Min(0x0000), Max(0x0064)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: Supported Max Codec Frames Per SDU: 0x01

Len: 0x01,  Metadata Length: 0x04
Len: 0x04,  Metadata:
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x01
                - Len: 0x02,  Value: Preferred Audio Contexts:  - If ASE is Src/Sink, Server is not available to transmit/receive audio for any Context Type value.(0x0000)

Codec Specific Capabilities PAC Record Index: 0x00
*********************************************************************
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
01 00 00 00                                        ....
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 4 Bytes --
-------------------------------------------------------------------
01 00 00 00                                        ....
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : PACS (0x1850)
[SECTION]     : PACS CLIENT EVENTS
[SUB-SECTION] : PACS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_CONTINUE (0x00C0)
[Event]       : GA_GET_CAPABILITIES_CNF (0x03)

-----------------------------------------------------------------
Char UUID: Sink Audio Locations (0x2BCA)
-----------------------------------------------------------------
Role: Sink(0x01)
Capability Type: Audio Locations (0x02)
Data Length: 0x04
Data:
Len: 0x04,  Audio Locations: 0x00000001
                - Front Left (0x1)
*********************************************************************
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
04 00 01 00                                        ....
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 4 Bytes --
-------------------------------------------------------------------
04 00 01 00                                        ....
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : PACS (0x1850)
[SECTION]     : PACS CLIENT EVENTS
[SUB-SECTION] : PACS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_CONTINUE (0x00C0)
[Event]       : GA_GET_CAPABILITIES_CNF (0x03)

-----------------------------------------------------------------
Char UUID: Supported Contexts (0x2BCE)
-----------------------------------------------------------------
Role: Sink(0x01)
Capability Type: Supported Contexts (0x03)
Data Length: 0x04
Data:
Len: 0x02,  Supported Sink Contexts: 0x04
        - Media (0x0004)
Len: 0x02,  Supported Source Contexts: 0x01
        - Unspecified (0x0001)
*********************************************************************
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 4 Bytes --
-------------------------------------------------------------------
04 00 01 00                                        ....
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 4 Bytes --
-------------------------------------------------------------------
04 00 01 00                                        ....
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : PACS (0x1850)
[SECTION]     : PACS CLIENT EVENTS
[SUB-SECTION] : PACS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_GET_CAPABILITIES_CNF (0x03)

-----------------------------------------------------------------
Char UUID: Available Contexts (0x2BCD)
-----------------------------------------------------------------
Role: Sink(0x01)
Capability Type: Available Contexts (0x04)
Data Length: 0x04
Data:
Len: 0x02,  Available Sink Contexts: 0x04
        - Media (0x0004)
Len: 0x02,  Available Source Contexts:0x01
        - Unspecified (0x0001)
*********************************************************************
1


    0. Exit.
    1. Refresh this Menu.

    2. GA Init.
    3. GA Setup.
    4. GA Shutdown.

    5. GA Register - PACS.
    6. Notify PACS - Audio Capability.
    7. Notify PACS - Available Audio Contexts.
    8. Notify PACS - Supported Audio Contexts.
    9. Notify PACS - Audio Locations.

   10. Setup PACS - Audio Role Discovery.
   11. Release PACS.
   12. Get PACS Capabilities - Audio Capability, Location, Supp/Avail Contexts

   20. Enable/Disable LE Advertising - Legacy. [CAP - Acceptor, GAP - Peripheral]
   21. Enable/Disable LE Advertising - Extended. [CAP - Acceptor, GAP - Peripheral]
   22. Enable/Disable LE Scan - Legacy. [CAP - Initiator, GAP - Central]
   23. Enable/Disable LE Scan - Extended. [CAP - Initiator GAP - Central]

   25. LE Create Connection. [CAP - Initiator GAP - Central]
   26. LE Extended Create Connection. [CAP - Initiator GAP - Central]
   27. LE Legacy Authenticate and Encrypt.
   28. LE LESC Authenticate and Encrypt.
   29. Disconnection.

   30. Display all Connected Devices.
   31. Set Connected Device Index.
   32. Remove Device from Device Manager.

   35. Configure Sink Audio Dump.
   36. Configure Source Audio Dump.

   40. Unicast Operations.
   45. Broadcast Operations.

   50. VCP Volume Controller Operations.
   51. VCP Volume Renderer Operations.

   55. MICP Microphone Controller Operations.
   56. MICP Microphone Device Operations.

   60. CSIP Set Coordinator and Set Member Operations.

   65. MCP Media Control Client.
   66. MCP Media Control Server.

   70. CCP Call Control Client.
   71. CCP Call Control Server.

   75. GA TMAP Operations.
   76. GA HAP Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

   90. Configure Notification

Your Option ?: 75

    0. Exit
    1. Refresh this Menu

    2. GA TMAP Initialize
    3. GA TMAP Register
    4. GA TMAP Shutdown

   10. Discover TMAS Role
   11. Release TMAS

   20. Unicast Client.
   21. VCP Volume Controller Operations.
   22. CCP Call Control Server.

   30. Unicast Server.
   31. VCP Volume Renderer Operations.

   40. Unicast Client - BAP Audio Source.
   41. VCP Volume Controller Operations.
   42. MCP Media Control Server.

   50. Unicast Server - BAP Audio Sink.
   51. VCP Volume Renderer Operations.

   60. Broadcast Source.

   70. Broadcast Sink.
   71. VCP Volume Renderer Operations.

   80. Get Context Info.
   81. Set Context Info.

   87. Free Saved Context Info.

Your Option? 40


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: 2
GA_sep_discover
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0xF0 with result 0x0000

-- Dumping 1 Bytes --
-------------------------------------------------------------------
4E                                                 N
-------------------------------------------------------------------

Received GATT_PS_DISCOVERY_RSP
No. Primary Services - 1

UUID: 0x184E (Audio Stream Control Service)
Start Hdl: 0x0080, End Hdl: 0x0089

[ATT]:[0x0D: 0x05]: Received ATT Event 0xF3 with result 0x0000

-- Dumping 3 Bytes --
-------------------------------------------------------------------
81 00 00                                           ...
-------------------------------------------------------------------

Received GATT_CHAR_DISCOVERY_RSP
No. Characteristics - 3

(ASE Control Point Characteristic)
Char Handle: 0x0081, UUID: 0x2BC6
Property: 0x1C, Value Handle: 0x0082
No. Characteristic Descriptors: 1
Desc Handle: 0x0083, Desc UUID: 0x2902 (CCCD)

(Audio Sink Endpoint Characteristic)
Char Handle: 0x0084, UUID: 0x2BC4
Property: 0x12, Value Handle: 0x0085
No. Characteristic Descriptors: 1
Desc Handle: 0x0086, Desc UUID: 0x2902 (CCCD)

(Audio Sink Endpoint Characteristic)
Char Handle: 0x0087, UUID: 0x2BC4
Property: 0x12, Value Handle: 0x0088
No. Characteristic Descriptors: 1
Desc Handle: 0x0089, Desc UUID: 0x2902 (CCCD)

[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x13 with result 0x0000
Received Write Response Opcode!
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 2 Bytes --
-------------------------------------------------------------------
01 00                                              ..
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_CONTINUE (0x00C0)
[Event]       : GA_SEP_DISCOVER_CNF (0x04)

-----------------------------------------------------------------
Char UUID: Sink ASE (0x2BC4)
-----------------------------------------------------------------
SEP Context ID - 0x00
Role: Sink(0x01)
Data Length: 0x02
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: Idle (0x00)
Length: 0x00,  Additional Parameters: NULL
*********************************************************************
[ATT]:[0x0D: 0x05]: Received ATT Event 0x0B with result 0x0000

-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------

Received Read Response Opcode!
Handle Value Received -

-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-SETUP
[TYPE]        : SETUP RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_DISCOVER_CNF (0x04)

-----------------------------------------------------------------
Char UUID: Sink ASE (0x2BC4)
-----------------------------------------------------------------
SEP Context ID - 0x01
Role: Sink(0x01)
Data Length: 0x02
Data:
Length: 0x01,  ASE ID: 0x02
Length: 0x01,  ASE State: Idle (0x00)
Length: 0x00,  Additional Parameters: NULL
*********************************************************************
3
Enter ASE Count: 1
Enter ASE ID:
1
Enter Role: 1 - Sink, 2 - Source:
1
Enter CS Conf Set ID:
Codec Specific Configuration:
Set ID.    Set Name:    Codec ID    Supp Sampling Freq    Supp Frame Dur    Supp Octets per codec frame    Bitrate
   1.       Set  8_1  :     LC3              8kHz              7.5ms                    26                 27.734kbps
   2.       Set  8_2  :     LC3              8kHz               10ms                    30                     24kbps
   3.       Set  16_1 :     LC3             16kHz              7.5ms                    30                     32kbps
   4.       Set  16_2 :     LC3             16kHz               10ms                    40                     32kbps
   5.       Set  24_1 :     LC3             24kHz              7.5ms                    45                     48kbps
   6.       Set  24_2 :     LC3             24kHz               10ms                    60                     48kbps
   7.       Set  32_1 :     LC3             32kHz              7.5ms                    60                     64kbps
   8.       Set  32_2 :     LC3             32kHz               10ms                    80                     64kbps
   9.       Set 441_1 :     LC3           44.1kHz              7.5ms                    97                  95.06kbps
  10.       Set 441_2 :     LC3           44.1kHz               10ms                   130                  95.55kbps
  11.       Set  48_1 :     LC3             48kHz              7.5ms                    75                     80kbps
  12.       Set  48_2 :     LC3             48kHz               10ms                   100                     80kbps
  13.       Set  48_3 :     LC3             48kHz              7.5ms                    90                     96kbps
  14.       Set  48_4 :     LC3             48kHz               10ms                   120                     96kbps
  15.       Set  48_5 :     LC3             48kHz              7.5ms                   117                  124.8kbps
  16.       Set  48_6 :     LC3             48kHz               10ms                   155                    124kbps
TMAP Role: UMS, M: 48_2, 48_4 OR 48_6
12
GA_sep_configure
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0x53 with result 0x0000

-- Dumping 32 Bytes --
-------------------------------------------------------------------
82 00 01 01 01 02 02 06 00 00 00 00 13 02 01 08    ................
02 02 01 05 03 01 00 00 00 03 04 64 00 02 05 01    ...........d....
-------------------------------------------------------------------

Received Write Command Tx Complete (Locally generated)

-- Dumping 32 Bytes --
-------------------------------------------------------------------
82 00 01 01 01 02 02 06 00 00 00 00 13 02 01 08    ................
02 02 01 05 03 01 00 00 00 03 04 64 00 02 05 01    ...........d....
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
82 00 01 01 01 00 00                               .......
-------------------------------------------------------------------

Received HVN
Handle - 0x0082
Handle Value Received -

-- Dumping 5 Bytes --
-------------------------------------------------------------------
01 01 01 00 00                                     .....
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 46 Bytes --
-------------------------------------------------------------------
85 00 01 01 00 02 0D 5F 00 40 9C 00 40 9C 00 00    ......._.@..@...
00 00 00 00 00 06 00 00 00 00 13 02 01 08 02 02    ................
01 05 03 01 00 00 00 03 04 64 00 02 05 01          .........d....
-------------------------------------------------------------------

Received HVN
Handle - 0x0085
Handle Value Received -

-- Dumping 44 Bytes --
-------------------------------------------------------------------
01 01 00 02 0D 5F 00 40 9C 00 40 9C 00 00 00 00    ....._.@..@.....
00 00 00 06 00 00 00 00 13 02 01 08 02 02 01 05    ................
03 01 00 00 00 03 04 64 00 02 05 01                .......d....
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-WRITE
[TYPE]        : WRITE RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_CONFIGURE_CNF (0x05)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
SEP Context ID - 0x00
Data Length: 0x2C
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: Codec Configured (0x01)
Length: 0x2A,  Additional Parameters:
        Len: 0x01,  Framing: Unframed ISOAL PDUs supported (0x00)
        Len: 0x01,  Preferred PHY: 0x02
                - LE 2M PHY preferred
        Len: 0x01,  Preferred Retransmission Number: 0x0D
        Len: 0x02,  Max Transport Latency: 0x005F(95ms)
        Len: 0x03,  Presentation Delay Min: 0x009C40(40000us)
        Len: 0x03,  Presentation Delay Max: 0x009C40(40000us)
        Len: 0x03,  Preferred Presentation Delay Min: No preference(0x000000)
        Len: 0x03,  Preferred Presentation Delay Max: No preference(0x000000)
        Len: 0x05,  Codec ID:
Len: 0x05,  Codec ID:
                - Len: 0x01,  Coding Format: LC3 (0x06),
                - Len: 0x02,  Company ID: (0x0000)
                - Len: 0x02,  Vendor Specific Codec ID: (0x0000)
        Len: 0x01,  Codec Specific Configuration Length: 0x13
        Len: 0x13,  Codec Specific Configuration:
                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x01
                - Len: 0x01,  Value: Sampling Frequencies: 48000 Hz (0x0008)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x02
                - Len: 0x01,  Value: Frame Durations: 10 ms (0x01)

                - Len: 0x01,  Length: 0x05
                - Len: 0x01,  Type: 0x03
                - Len: 0x04,  Value: Audio Channel Allocation:
                - Front Left (0x1)

                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x04
                - Len: 0x02,  Value: Octets per Codec Frame: 0x0064

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: Codec Frame Blocks Per SDU: 0x01

*********************************************************************
4
Since SDU Interval, SCA, Packing, Framing, Max Transport latency are to be common for a CIG, Ensure the selection of  Set ID results in common values for below
For simplicity, Configuring all CISes of the CIG with the same set
Call this separately if CIS of different Conf need to be configured
Enter CIG ID:
CIG_ID: 0x00 - 0xEF
1
Enter CIS Count: 1
Enter SCA: 251 ppm to 500 ppm: 0x00
151 ppm to 250 ppm: 0x01
101 ppm to 150 ppm: 0x02
76 ppm to 100 ppm : 0x03
51 ppm to 75 ppm  : 0x04
31 ppm to 50 ppm  : 0x05
21 ppm to 30 ppm  : 0x06
0 ppm to 20 ppm   : 0x07
0
Enter Packing:
Packing: Sequential - 0x00
Packing: Interleaved - 0x01
0
Ensure that the Set ID input here is the same as used during Config Codec
Enter QoS Codec Capability/Configuration Setting Set ID:
QoS Codec Capability/Configuration Setting
NOTE: Ensure to enter Set ID corresponding to Records added in PACS.
Set ID.    Set Name:    Codec Cap/Conf Setting    SDU Interval    Framing    Max SDU Size    Retransmission Num    Max Transport Latency    Presentation Delay
QoS Capability/Conf settings for low latency audio data:
   1.      Set  8_1_1  :          8_1                7500us        unframed         26                 2                     8ms                40000us
   2.      Set  8_2_1  :          8_2               10000us        unframed         30                 2                    10ms                40000us
   3.      Set  16_1_1 :         16_1                7500us        unframed         30                 2                     8ms                40000us
   4.      Set  16_2_1 :         16_2               10000us        unframed         40                 2                    10ms                40000us
   5.      Set  24_1_1 :         24_1                7500us        unframed         45                 2                     8ms                40000us
   6.      Set  24_2_1 :         24_2               10000us        unframed         60                 2                    10ms                40000us
   7.      Set  32_1_1 :         32_1                7500us        unframed         60                 2                     8ms                40000us
   8.      Set  32_2_1 :         32_2               10000us        unframed         80                 2                    10ms                40000us
   9.      Set 441_1_1 :        441_1                7500us          framed         97                 5                    24ms                40000us
  10.      Set 441_2_1 :        441_2               10000us          framed        130                 5                    31ms                40000us
  11.      Set  48_1_1 :         48_1                7500us        unframed         75                 5                    15ms                40000us
  12.      Set  48_2_1 :         48_2               10000us        unframed        100                 5                    20ms                40000us
  13.      Set  48_3_1 :         48_3                7500us        unframed         90                 5                    15ms                40000us
  14.      Set  48_4_1 :         48_4               10000us        unframed        120                 5                    20ms                40000us
  15.      Set  48_5_1 :         48_5                7500us        unframed        117                 5                    15ms                40000us
  16.      Set  48_6_1 :         48_6               10000us        unframed        155                 5                    20ms                40000us
QoS Capability/Conf settings for high-reliability audio data:
  17.      Set  8_1_2  :          8_1                7500us        unframed         26                13                    75ms                40000us
  18.      Set  8_2_2  :          8_2               10000us        unframed         30                13                    95ms                40000us
  19.      Set  16_1_2 :         16_1                7500us        unframed         30                13                    75ms                40000us
  20.      Set  16_2_2 :         16_2               10000us        unframed         40                13                    95ms                40000us
  21.      Set  24_1_2 :         24_1                7500us        unframed         45                13                    75ms                40000us
  22.      Set  24_2_2 :         24_2               10000us        unframed         60                13                    95ms                40000us
  23.      Set  32_1_2 :         32_1                7500us        unframed         60                13                    75ms                40000us
  24.      Set  32_2_2 :         32_2               10000us        unframed         80                13                    95ms                40000us
  25.      Set 441_1_2 :        441_1                7500us          framed         97                13                    80ms                40000us
  26.      Set 441_2_2 :        441_2               10000us          framed        130                13                    85ms                40000us
  27.      Set  48_1_2 :         48_1                7500us        unframed         75                13                    75ms                40000us
  28.      Set  48_2_2 :         48_2               10000us        unframed        100                13                    95ms                40000us
  29.      Set  48_3_2 :         48_3                7500us        unframed         90                13                    75ms                40000us
  30.      Set  48_4_2 :         48_4               10000us        unframed        120                13                   100ms                40000us
  31.      Set  48_5_2 :         48_5                7500us        unframed        117                13                    75ms                40000us
  32.      Set  48_6_2 :         48_6               10000us        unframed        155                13                   100ms                40000us
TMAP Role: UMS, M: Low Latency: Set 16_2_1, 48_2_1, 48_4_1, 48_6_1                                     High Reliability: Set 48_2_2, 48_4_2, 48_6_2
TMAP Role: UMR, M: Low Latency: Set 16_2_1, 24_2_1, 48_1_1, 48_2_1, 48_3_1, 48_4_1, 48_5_1, 48_6_1                                     High Reliability: Set 16_2_2, 24_2_2, 48_1_2, 48_2_2, 48_3_2, 48_4_2, 48_5_2, 48_6_2
TMAP Role: BMR, M: Low Latency: Set 16_2_1, 24_2_1, 48_1_1, 48_2_1, 48_3_1, 48_4_1, 48_5_1, 48_6_1                                     High Reliability: Set 16_2_2, 24_2_2, 48_1_2, 48_2_2, 48_3_2, 48_4_2, 48_5_2, 48_6_2
28
Enter the Device to which CIS needs to be created:
Device:
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
Connection Handle: 0x0000
Initiator: 0x01
Index: 0x01

Enter the Conn device index: 1
Enter CIS ID:
CIS_ID: 0x00 - 0xEF
1
Enter Direction:
1. Unidirectional
2. Bidirectional
1
Enter ASE ID:
1
Set CIG Params...
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2062 -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 4 Bytes --
-------------------------------------------------------------------
01 01 02 00                                        ....
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2062 -> HCI_LE_SET_CIG_PARAMETERS_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 4 Bytes --
-------------------------------------------------------------------
01 01 02 00                                        ....
-------------------------------------------------------------------


CIG ID: 1
CIS Count: 1
CIS Conn Handle: 0x0002
1


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: 5
Enter ASE Count: 1
Enter ASE ID:
1
GA_sep_setup
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0x53 with result 0x0000

-- Dumping 20 Bytes --
-------------------------------------------------------------------
82 00 02 01 01 01 01 10 27 00 00 02 64 00 0D 5F    ........'...d.._
00 40 9C 00                                        .@..
-------------------------------------------------------------------

Received Write Command Tx Complete (Locally generated)

-- Dumping 20 Bytes --
-------------------------------------------------------------------
82 00 02 01 01 01 01 10 27 00 00 02 64 00 0D 5F    ........'...d.._
00 40 9C 00                                        .@..
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
82 00 02 01 01 00 00                               .......
-------------------------------------------------------------------

Received HVN
Handle - 0x0082
Handle Value Received -

-- Dumping 5 Bytes --
-------------------------------------------------------------------
02 01 01 00 00                                     .....
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 19 Bytes --
-------------------------------------------------------------------
85 00 01 02 01 01 10 27 00 00 02 64 00 0D 5F 00    .......'...d.._.
40 9C 00                                           @..
-------------------------------------------------------------------

Received HVN
Handle - 0x0085
Handle Value Received -

-- Dumping 17 Bytes --
-------------------------------------------------------------------
01 02 01 01 10 27 00 00 02 64 00 0D 5F 00 40 9C    .....'...d.._.@.
00                                                 .
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-WRITE
[TYPE]        : WRITE RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_SETUP_CNF (0x06)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
SEP Context ID - 0x00
Data Length: 0x11
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: QoS Configured (0x02)
Length: 0x0F,  Additional Parameters:
        Len: 0x01,  CIG ID: 0x01
        Len: 0x01,  CIS ID: 0x01
        Len: 0x01,  SDU Interval: 0x002710
        Len: 0x01,  Framing: Unframed (0x00)
        Len: 0x01,  PHY: 0x02
                - LE 2M PHY preferred
        Len: 0x01,  MAX SDU: 0x000064
        Len: 0x01,  Retransmission Number: 0x0D
        Len: 0x02,  Max Transport Latency: 0x005F(95ms)
        Len: 0x03,  Presentation Delay: 0x009C40(40000us)
*********************************************************************
1


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: 6
Enter ASE Count: 1
Enter ASE ID:
1
Supported Sink Contexts: 0x04
        - Media (0x0004)
Available Sink Contexts: 0x04
        - Media (0x0004)
Enter CCID Count:
NOTE: Max Limit for CCID count is 15
1
Enter CCID:
The CCID_List LTV structure is used as part of the information supplied by the Initiator in the Metadata field associated with the Audio Streams during the Setting Context Type and CCID values Metadata step of the Unicast/Broadcast Audio Start procedure (see Section 7.3.1.2.6/Section 7.3.1.3.2) or the Updating Context Type and CCID Value Metadata of the Unicast/Broadcast Audio Update procedure (see Section 7.3.1.3.2/Section 7.3.1.6.1).
Refer CAP Spec V 1.0
1
GA_sep_start
Retval - GA_SUCCESS (0x0000)


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: [ATT]:[0x0D: 0x05]: Received ATT Event 0x53 with result 0x0000

-- Dumping 13 Bytes --
-------------------------------------------------------------------
82 00 03 01 01 07 03 02 04 00 02 05 01             .............
-------------------------------------------------------------------

Received Write Command Tx Complete (Locally generated)

-- Dumping 13 Bytes --
-------------------------------------------------------------------
82 00 03 01 01 07 03 02 04 00 02 05 01             .............
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
82 00 03 01 01 00 00                               .......
-------------------------------------------------------------------

Received HVN
Handle - 0x0082
Handle Value Received -

-- Dumping 5 Bytes --
-------------------------------------------------------------------
03 01 01 00 00                                     .....
-------------------------------------------------------------------

[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 14 Bytes --
-------------------------------------------------------------------
85 00 01 03 01 01 07 03 02 04 00 02 05 01          ..............
-------------------------------------------------------------------

Received HVN
Handle - 0x0085
Handle Value Received -

-- Dumping 12 Bytes --
-------------------------------------------------------------------
01 03 01 01 07 03 02 04 00 02 05 01                ............
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-WRITE
[TYPE]        : WRITE RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_START_CNF (0x07)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
SEP Context ID - 0x00
Data Length: 0x0C
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: Enabling (0x03)
Length: 0x0A,  Additional Parameters:
        Len: 0x01,  CIG ID: 0x01
        Len: 0x01,  CIS ID: 0x01
        Len: 0x07,  Metadata Length: 0x07
        Len: 0x07,  Metadata:
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x02
                - Len: 0x02,  Value: Streaming Audio Contexts:  - Media (0x0004)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: CCID List: 0x1

Creating CIS
*********************************************************************
Received HCI_COMMAND_STATUS_EVENT.
        Command Status: 0x00
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x2064 (UNKNOWN)

Received HCI_COMMAND_STATUS_EVENT In LE.
        LE Command Status: 0x00
        LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x2064 (HCI_LE_CREATE_CIS_OPCODE)
Subevent : HCI_LE_CIS_ESTABLISHED_SUBEVENT.
Status: 0x00
Connection Handle: 0x0002
CIG_Sync_Delay: 0x0000100E
CIS_Sync_Delay: 0x0000100E
Transport_Latency_M_To_S: 0x00000005
Transport_Latency_S_To_M: 0x00000005
PHY_M_To_S: 0x02
PHY_S_To_M: 0x02
[APPL][GA][UCC][SRC]: ISO Data Path Setup Status: In-progress
Received HCI_COMMAND_COMPLETE_EVENT.
        Num Command Packets: 1 (0x01)
        Command Opcode: 0x206E -> UNKNOWN
        Command Status: 0x00
        Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------



Received HCI_COMMAND_COMPLETE_EVENT In LE.
         LE Num Command Packets: 1 (0x01)
        LE Command Opcode: 0x206E -> HCI_LE_SETUP_ISO_DATA_PATH_OPCODE
        LE Command Status: 0x00
        LE Return Parameters:
-- Dumping 2 Bytes --
-------------------------------------------------------------------
02 00                                              ..
-------------------------------------------------------------------


[APPL][GA][UCC][SRC]: ISO Data Path Setup Status: Complete
[ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 14 Bytes --
-------------------------------------------------------------------
85 00 01 04 01 01 07 03 02 04 00 02 05 01          ..............
-------------------------------------------------------------------

Received HVN
Handle - 0x0085
Handle Value Received -

-- Dumping 12 Bytes --
-------------------------------------------------------------------
01 04 01 01 07 03 02 04 00 02 05 01                ............
-------------------------------------------------------------------

************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-NOTIFICATION
[TYPE]        : NOTIFICATION
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_RX_START_READY_NTF (0x24)
[APPL][GA][UCC][SRC]: LC3 Encoder Setup Status: Created
[APPL][GA][UCC][SRC]: Audio PL Generator Setup Status: Success
[APPL][GA][UCC][SRC]: Audio PL Generator Start Process Status: Success

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
SEP Context ID - 0x00
Data Length: 0x0C
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: Streaming (0x04)
Length: 0x0A,  Additional Parameters:
        Len: 0x01,  CIG ID: 0x01
        Len: 0x01,  CIS ID: 0x01
        Len: 0x07,  Metadata Length: 0x07
        Len: 0x07,  Metadata:
                - Len: 0x01,  Length: 0x03
                - Len: 0x01,  Type: 0x02
                - Len: 0x02,  Value: Streaming Audio Contexts:  - Media (0x0004)

                - Len: 0x01,  Length: 0x02
                - Len: 0x01,  Type: 0x05
                - Len: 0x01,  Value: CCID List: 0x1

*********************************************************************
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 1* * * * * * * * * * * *


    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 9* * * * * * * * * * * * * * * * * * * * * * *
Enter ASE Count: * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 1* * * * * * * * * * * * * *
Enter ASE ID:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 1* * * * * * * * * * * * * *
GA_sep_suspend
Retval - GA_SUCCESS (0x0000)
[ATT]:[0x0D: 0x05]: Received ATT Event 0x53 with result 0x0000

-- Dumping 5 Bytes --
-------------------------------------------------------------------
82 00 05 01 01                                     .....
-------------------------------------------------------------------

Received Write Command Tx Complete (Locally generated)

-- Dumping 5 Bytes --
-------------------------------------------------------------------
*

    0. Exit.
    1. Refresh this Menu.

    2. ASE Discover.
    3. ASE Codec Configuration.
    4. ASE Setup CIG Parameters - HCI.
    5. ASE QoS Configuration.
    6. ASE Enable.
    7. ASE Receiver Start Ready for Local ASE Sink.
    8. ASE Update.
    9. ASE Disable.
   10. ASE Receiver Stop Ready for Local ASE Sink.
   11. ASE Release.

   15. Display Remote ASEs Data.

   16. ASE Create CIS.
   17. ASE Disconnect CIS.
   18. Remove CIG

   19. Display all Connected Devices.

Your Option ?: 82 00 05 01 01                                     .....
-------------------------------------------------------------------

* * * * * * * * * * * * * * * * * * * [ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 7 Bytes --
-------------------------------------------------------------------
* 82 00 05 01 01 00 00                               .......
-------------------------------------------------------------------

Received HVN
Handle - 0x0082
Handle Value Received -

-- Dumping 5 Bytes --
-------------------------------------------------------------------
* 05 01 01 00 00                                     .....
-------------------------------------------------------------------

* * * * * * * * [ATT]:[0x0D: 0x05]: Received ATT Event 0x1B with result 0x0000

-- Dumping 19 Bytes --
-------------------------------------------------------------------
85 00 01 02 01 01 10 27 00 00 02 64 00 0D 5F 00    .......'...d.._.
* 40 9C 00                                           @..
-------------------------------------------------------------------

Received HVN
Handle - 0x0085
Handle Value Received -

-- Dumping 17 Bytes --
-------------------------------------------------------------------
* 01 02 01 01 10 27 00 00 02 64 00 0D 5F 00 40 9C    .....'...d.._.@.
00                                                 .
-------------------------------------------------------------------

* ************************** CALLBACK: GA Client **************************
[Profile]     : ASCS (0x184E)
[SECTION]     : ASCS CLIENT EVENTS
[SUB-SECTION] : ASCS CLIENT-WRITE
[TYPE]        : WRITE RESPONSE
[BD ADDRESS]  : 0x16 0x9C 0x01 0xE8 0x07 0xC0
[BD_ADDR TYPE]: 0x00
[Event Status]: GA_SUCCESS (0x0000)
[Event]       : GA_SEP_SUSPEND_CNF (0x08)

-----------------------------------------------------------------
Char UUID: ASE Control Point (0x2BC6)
-----------------------------------------------------------------
SEP Context ID - 0x00
Data Length: 0x11
Data:
Length: 0x01,  ASE ID: 0x01
Length: 0x01,  ASE State: QoS Configured (0x02)
Length: 0x0F,  Additional Parameters:
        Len: 0x01,  CIG ID: 0x01
        Len: 0x01,  CIS ID: 0x01
        Len: 0x01,  SDU Interval: 0x002710
        Len: 0x01,  Framing: Unframed (0x00)
        Len: 0x01,  PHY: 0x02
                - LE 2M PHY preferred
        Len: 0x01,  MAX SDU: 0x000064
        Len: 0x01,  Retransmission Number: 0x0D
        Len: 0x02,  Max Transport Latency: 0x005F(95ms)
        Len: 0x03,  Presentation Delay: 0x009C40(40000us)
[APPL][GA][UCC][SRC]: LC3 Encoder Setup Status: Deleted
[APPL][GA][UCC][SRC]: Audio PL Generator Stop Process Status: Success
*********************************************************************
###################################








































Steps to for MCP(Media Control Profile):
-----------------------------------------
A) Initialization of BAP, CAP and GA Bearer Layer.
   2.  EtherMind Init.
   3.  Bluetooth ON.
   200. GA Profile Option.
   2.  GA Init.
   0. Return to main menu

B) For MCP Server Setup:
   200. GA Profile Option.
    50. LE Adv. Enable.
    <<Advertise the device for connection>>

   0. Return to main menu

   204. GA MCP Server Operations.
   5.  GA MCS Initialize   <- To Initialize MCP Server Module
   11. Add MCS Instance: <- To register MCS service
       Enter Media Player to be added: [1: VLC] [2:MPC] 2
    9.  Get Remote Dev addr <- Register Peer Address
    0. Return to main menu

   210. GA Setup.
     <<Dynamic DB Setup>>

C) Extended Connection Initiate and SMP Authenticate (From MCP client).
   60. Set peer endpoint
   Enter endpoint address: xx xx xx xx xx xx
   Enter endpoint type: 0
   52. LE Extended Connection

   Verify below and take note of "Connection Handle"
   Received HCI_LE_META_EVENT.
   Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
   Connection Handle: 0x0000
   Channel Selection Algorithm: 0x01

   55. LE Authenticate and Encrypt
   Verify:
   Recvd SMP_AUTHENTICATION_COMPLETE
   Status : 0000

   0. Return to main menu

<<Now, BLE Connection & Authentication is done, ready to perform MCP operations>>

D) For MCP Client Setup:
   203. GA MCP Client Operations.
    5.  GA MCP CE Init
    11. Setup GMCS session with Peer Device
        Enter Peer BD Address: xx xx xx xx xx xx
        Enter endpoint type: x
    <Wait for Context Setup Complete: GMCS Instance: 0x00>
    12. Discover MCS instances on remote device
        Enter Peer BD Address: xx xx xx xx xx xx
        Enter endpoint type: 0
    <Wait for MCS Service Discovery Result: Success>
       GA Status: 0x00C0 <- Continue response.
       <Note down the Start & End Handles of MCS Service>
       <Example:
        MCS -> Start Handle:0x00AD End Handle:0x00EA>
       GA Status: 0x0000 <- Final response.

    13. Setup MCS session with Peer Device
        <Enter correct handles of MCS>
       Enter start handle(in Hex):
       Enter end handle(in Hex):
       <Wait for Context Setup Complete: MCS Instance: 0x00>

E) For MCP Client Operations:
   - Select appropriate menu options and wait for confirm event.

   Example:
   17. Set the Instance
   Session Instance Type: [MCS:0] [GMCS: 1]
    1
    Session Instance ID:
    0

   18. Config all Char
    Configuration Notificaton : [Disable:0] [Enable: 1]
    1
    Wait for 'MCP_CE_ENABLE_ALL_CFG_CNF' Event.

   20. Read Media Player Name
      Wait for 'MCP_CE_READ_MEDIA_PLAYER_NAME_CNF' Event.

   25. Read Track Title
      Wait for 'MCP_CE_READ_TRACK_TITLE' Event.

F) For MCP Server Operations:
   - Select appropriate menu options for Sending the Notification,
   Example:
    13. Notify Media Player Name
      - Check for 'MCP_CE_MEDIA_PLAYER_NAME_NTF' event at the MCP Server.

    14. Notify Track Title
      - Check for 'MCP_CE_TRACK_TITLE_NTF' event at the MCP Server.

G) To release the service at MCP Client.
   18. Config all Char
       Configuration Notificaton : [Disable:0] [Enable: 1]
       0
       Wait for 'MCP_CE_DISABLE_ALL_CFG_CNF'

   19. Release GMCS And/Or MCS


Steps to setup GA Broadcast Streaming:

Initialization of Stack and BAP on Broadcast Source and Sink:
   2.  EtherMind Init.
   3.  Bluetooth ON.
   200. GA Profile Option.
   2.  GA Init.

Broadcast Source:
A) Allocate and Configure Broadcast session on source.
   80. Allocate Session
   82. Configure Session

B) Register Source Endpoint with CodecInfo, Metadata, NumStreams etc to configured session
   83. Register Source Endpoint

C) Advertise Announcements (Broadcast and Basic)
   84. Setup Announcement

Verify:
   GA_BC_SETUP_ANNOUNCEMENT_CNF with Status 0x0000

D) Start Broadcast Streaming
   86. Start Broadcast

Verify:
   GA_BC_START_CNF with Status 0x0000

E) Stop Broadcast Streaming and Stop Advertising Announcements
   87. Stop Broadcast
   85. End Announcement

Verify:
   GA_BC_SUSPEND_CNF with Status 0x0000
   GA_BC_END_ANNOUNCEMENT_CNF with Status 0x0000

Broadcast Sink:
A) Scan for available broadcast announcements
   90. Scan Announcement

Verify:
   GA_BC_SCAN_ANNOUNCEMENT_CNF with Status 0x0000

Verify:
   GA_BC_SOURCE_ANNOUNCEMENT_IND

B) Synchronize to the Advertisements in the Announcements
   92. Associate Stream
       Enter SID: << Enter SID of the desired announcement displayed >>
       Enter endpoint address: << Enter the BD Address of the announcement source >>
       Enter endpoint type: << Enter the BD Address Type of the announcement source >>

Verify:
   GA_BC_ASSOCIATE_CNF with Status 0x0000

Verify:
   GA_BC_SOURCE_CONFIG_IND

Verify:
   GA_BC_SOURCE_STREAM_IND

C) Synchronize to the Broadcast stream and Start streaming
   94. Enable Stream

Verify:
   GA_BC_ENABLE_CNF with Status 0x0000

D) Disable Stream and Dissociate from Source
   95. Disable Stream
   93. Dissociate Stream

Verify:
   GA_BC_DISABLE_CNF with Status 0x0000

Verify:
   GA_BC_DISSOCIATE_CNF with Status 0x0000


Steps to for MICP(Microphone Control Profile):
-----------------------------------------
A) Initialization of BAP, CAP and GA Bearer Layer.
MICP Controller and MICP Device Init:
   2.  EtherMind Init.
   3.  Bluetooth ON.
 200. GA Profile Option.
   2.  GA Init.

   0. Return to main menu

 210. GA Setup.
     <<Dynamic DB Setup>>

B) For MICP Device Setup:
   200. GA Profile Option.
   50. LE Adv. Enable.
    <<Advertise the device for connection>>

   43. MICP Device Operations.
   30. Set the Peer Address
   Enter Peer BD Address: xx xx xx xx xx xx <Enter MICP Controller BD Address>
   Enter endpoint type: 0

C) For MICP Controller Setup:
   200. GA Profile Option.
   60. Set peer endpoint
   Enter endpoint address: xx xx xx xx xx xx <Enter MICS Device BD Address>
   Enter endpoint type: 0

   52. LE Extended Connection
   Verify below and take note of "Connection Handle"
   Received HCI_LE_META_EVENT.
   Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
   Connection Handle: 0x0000
   Channel Selection Algorithm: 0x01

   55. LE Authenticate and Encrypt
    Verify:
    Recvd SMP_AUTHENTICATION_COMPLETE
    Status : 0000

<<Now, BLE Connection & Authentication is done, ready to perform MICP operations>>

D) For MICP Controller Setup:
   42. MICP Controller Operations.
    2. MICP Setup
       <Enter Peer Address>
      Wait for GA_MC_SETUP_CNF

    3. MICP Get Capabilities <= To discover included service
       Wait for GA_MC_GET_CAPABILITIES_CNF event.
       GA Status: 0xC0 <- Continue response.
       GA Status: 0x00 <- Final response.
       <Note down the Start & End Handles of AICS service>
      Example:
      AICS -> Start Handle = 0x49, End Handle: 0x58

    4. MICP Setup AICS Capability <- To setup the AICS
       - Enter Valid Start/End Handles of the AICS service
        <AICS Start Handle:0x0049
         AICS End Handle:0x0058>
        Wait for GA_MC_SET_CAPABILITIES_CNF event

E) For MICP Controller operation:
   - Select appropriate menu options and wait for confirm event.

   Example:
   10. MICS Get Mute
      Wait for 'GA_MC_GET_MUTE_CNF' Event.

   11. MICS Set Mute
      Select your choice
      0 -> Not Mute
      1 -> Mute
      2 -> Disable Mute
      1
      Wait for 'GA_MC_SET_MUTE_CNF' event
      Wait for 'GA_MC_MUTE_NTF' Event.

   30. MICS AICS Get Audio Input State
      Wait for 'GA_MC_AICS_GET_INPUT_STATE_CNF' Event

   36. MICS AICS Get Audio Input State
      - Enter Gain Setting(-128 to 127): 10
        Wait for 'GA_MC_AICS_CP_WT_CNF' Event
        Wait for 'GA_MC_AICS_INPUT_STATE_NTF' Event

F) For MICP Device operations:
   - Select appropriate menu options for sending the notification.

   Example:
   50. MICS Notify Mute
     - Check 'GA_MC_MUTE_NTF' event at MICP Controller.
   51. AICS Notify Audio Input State
     - Check 'GA_MC_AICS_INPUT_STATE_NTF' event at MICP Controller.

G) To release the service at MICP Controller
   6. MICP Release AICS Capability
    - Wait for 'GA_MC_RELEASE_CAPABILITY_CNF' event
   5. MICP Release MICS
   - Wait for 'GA_MC_RELEASE_CNF' event



Steps to for VCP(Volume Control Profile):
-----------------------------------------
A) Initialization of BAP, CAP and GA Bearer Layer.
VCP Controller and VCP Renderer Init:
   2.  EtherMind Init.
   3.  Bluetooth ON.
 200. GA Profile Option.
   2.  GA Init.

   0. Return to main menu

 210. GA Setup.
     <<Dynamic DB Setup>>

B) For VCP Renderer Setup:
   200. GA Profile Option.
   50. LE Adv. Enable.
    <<Advertise the device for connection>>

   41. VCP Renderer Operations.
   7. Register Peer Device Address.
   Enter Peer BD Address:  xx xx xx xx xx xx <Enter VCP Renderer BD Address>
   Enter endpoint type: 0

C) Extended Connection Initiate and SMP Authenticate (From VCP Controller).
   200. GA Profile Option.
   60. Set peer endpoint
   Enter endpoint address: xx xx xx xx xx xx <Enter VCP Renderer BD Address>
   Enter endpoint type: 0
   52. LE Extended Connection

   Verify below and take note of "Connection Handle"
   Received HCI_LE_META_EVENT.
   Subevent : HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVENT.
   Connection Handle: 0x0000
   Channel Selection Algorithm: 0x01

   55. LE Authenticate and Encrypt
    Verify:
    Recvd SMP_AUTHENTICATION_COMPLETE
    Status : 0000

<<Now, BLE Connection & Authentication is done, ready to perform VCP operations>>

D) For VCP Controller Setup:
   40. VCP Controller Operations.
    2. VCP Setup
       <Enter Peer Address>
      Wait for GA_VC_SETUP_CNF

    3. VCP Get Capabilities <= To discover included services
       Wait for GA_VC_GET_CAPABILITIES_CNF event.
       GA Status: 0xC0 <- Continue response.
       GA Status: 0x00 <- Final response.
       <Note down the Start & End Handles of VOCS/AICS services>
       Example:
       VOCS -> Start Handle = 0x22, End Handle: 0x2D
       AICS -> Start Handle = 0x2E, End Handle: 0x3D

    4. VCP Setup VOCS Capability <- To setup the VOCS
       - Enter Valid Start/End Handles of the VOCS service
         Wait for GA_VC_SET_CAPABILITY_CNF event

    5. VCP Setup AICS Capability <- To setup the AICS
       - Enter Valid Start/End Handles of the AICS service
         Wait for GA_VC_SET_CAPABILITY_CNF event

E) For VCP Controller operation:
   - Select appropriate menu options and wait for confirm event.

   Example:
   10. VCS Get Volume State
      Wait for 'GA_VC_GET_VOLUME_STATE_CNF' Event.

   12. VCS Send Relative Volume Down
      Wait for 'GA_VC_CP_WT_CNF' event
      Wait for 'GA_VC_VOLUME_STATE_NTF' Event.

   20. VOCS Get Volume Offset
      Wait for 'GA_VC_VOCS_GET_OFFSET_STATE_CNF' Event.

   23. VOCS Set Volume Offset
      - Enter Volume Offset Value: 10
      Wait for GA_VC_VOCS_CP_WT_CNF
      Wait for GA_VC_VOCS_OFFSET_STATE_NTF

   30. AICS Get Audio Input State
      Wait for 'GA_VC_AICS_GET_INPUT_STATE_CNF' Event

   36. AICS Set Gain Setting
      - Enter Gain Setting(-128 to 127): 10
        Wait for 'GA_VC_AICS_CP_WT_CNF' Event
        Wait for 'GA_VC_AICS_INPUT_STATE_NTF' Event

F) For VCP Renderer operations:
   - Select appropriate menu options for sending the notification.

   Example:
   50. VCS Notify Volume State
     - Check 'GA_VC_VOLUME_STATE_NTF' event at VCP Controller.
   52. VOCS Notify Volume Offset State
     - Check 'GA_VC_VOCS_OFFSET_STATE_NTF' event at VCP Controller.
   55. AICS Notify Audio Input State
     - Check 'GA_VC_AICS_INPUT_STATE_NTF' event at VCP Controller.

G) To release the service at VCP Controller
   8. VCP release AICS
    - Wait for 'GA_VC_RELEASE_CAPABILITY_CNF' event
   7. VCP release VOCS
    - Wait for 'GA_VC_RELEASE_CAPABILITY_CNF' event
   6. VCP release VCS
    - Wait for 'GA_VC_RELEASE_CNF' event

BT only firmware download :
-----------------------------------------
    - In order to download BT only firmware, rebuild the code by adding CONFIG_BT_IND_DNLD flag
    - BT only firmware will be automatically downlaod on boot.
    
    Example:
     Initialize 1XK_USD_Murata Driver
    download starts(155712)
    ......................................................................................................................................................................
    download success!
    host init done

BT Only Independent Reset :
-----------------------------------------
    - Make sure code is complied with CONFIG_BT_IND_DNLD flag
    - IR need to be configured everytime after firmware is downloaded/redownloaded
    - Make sure to turn-on BT again (option-3) after trigerring IR

    Out of Band
    ------------
    2.  EtherMind Init.
    3.  Bluetooth ON.
    255. Configure IR Inband/Outband.
    select IR mode to configure (0:Disable, 1:Outband, 2:Inband):1
    IR configured successfully for mode 1, ir_state = 8
    256. Trigger IR & BT-FW Download.
    Initiating IR trigger!!
    EtherMind: Bluetooth OFF ...
    Sending Out of Band IR Trigger
    download starts(210332)
    ..............................................................................................................
    download success!
    IR exit with state = 0
    *** select option-3 to turn-on BT again!!***

    Inband
    ------
    2.  EtherMind Init.
    3.  Bluetooth ON.
    255. Configure IR Inband/Outband.
    select IR mode to configure (0:Disable, 1:Outband, 2:Inband):2
    IR configured successfully for mode 2, ir_state = 8
    256. Trigger IR & BT-FW Download.
    Initiating IR trigger!!
    EtherMind: Bluetooth OFF ...
    Sending Inband IR Trigger
    download starts(210332)
    ..............................................................................................................
    download success!
    IR exit with state = 0
    *** select option-3 to turn-on BT again!!***
