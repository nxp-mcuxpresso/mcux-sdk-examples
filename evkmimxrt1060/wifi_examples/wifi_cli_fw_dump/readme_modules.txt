Supported Wi-Fi/BT/BLE modules
==============================
  - AzureWave AW-NM191NF-uSD
  - AzureWave AW-AM510-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2DS M.2 Module (EAR00386)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - u-blox EVK-LILY-W131/-W132 (with SD-uSD adapter, for example Delock 61680)
  - u-blox EVK-JODY-W263
  - u-blox EVK-MAYA-W161/-W166


Murata Solution Board settings
==============================
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 2DS module datasheet: https://www.embeddedartists.com/doc/ds/2DS_M2_Datasheet.pdf
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
 - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and WLAN-CTRL = 3.3V
 - J1 = 2-3: 3.3V from uSD connector


Modules Settings
================
Jumper settings on AzureWave AW-NM191NF-uSD Module:
  - J11 1-2: VIO_SD 1.8V (Voltage level of SDIO pins is 1.8V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)

Jumper settings on AzureWave AW-AM510-uSD Module:
  - J4  1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - The pin 1 of J4 is not marked on the board. Please note that pin numbering of J4 is opposite to J2 (pin 1 is close to the "J4" label):
         3 2 1
         o o=o J4
      J2 o=o o
         1 2 3

Jumper settings on AzureWave AW-CM358-uSD Module:
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - The pin 1 of J4 is not marked on the board. Please note that pin numbering of J4 is opposite to J2 (pin 1 is close to the "J4" label):
         3 2 1
         o o=o J4
      J2 o=o o
         1 2 3


u-blox Modules Settings
=======================
Jumper settings on u-blox EVK-LILY-W131/-W132:
  - J2 7-8: SDIO interface selection
  - J3 3-4: VIO 3.3V (Voltage level of SDIO pins is 3.3V)
  - J3 5-6: 3.3V Power Supply from uSD connector
User Guide: https://www.u-blox.com/sites/default/files/EVK-LILY-W1_UserGuide_%28UBX-15030290%29.pdf

Jumper settings on u-blox EVK-JODY-W263:
  - SW503 = 1101: SDIO-UART interface selection
  - J104/J105 3V3_IN-SDIO: 3.3V Power Supply from uSD connector
User Guide: https://www.u-blox.com/sites/default/files/EVK-JODY-W2_UserGuide_UBX-19027118.pdf

Jumper settings on u-blox EVK-MAYA-W161/-W166:
  - J13 1-2: SDIO-UART interface selection
  - J7  1-2,3-4: 3.3V Power Supply from uSD connector
  - J8  3-4,7-8: VIO/VIO_SD 1.8V (Voltage level of SDIO pins is 1.8V)
User Guide: https://www.u-blox.com/sites/default/files/EVK-MAYA-W1_UserGuide_UBX-21039658.pdf

