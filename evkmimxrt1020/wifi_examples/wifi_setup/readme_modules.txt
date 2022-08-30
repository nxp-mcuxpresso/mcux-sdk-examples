Supported Wi-Fi/BT/BLE modules
==============================
  - AzureWave AW-NM191NF-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2DS M.2 Module (EAR00386)


Murata Solution Board settings
==============================
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 2DS module datasheet: https://www.embeddedartists.com/doc/ds/2DS_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
 - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and WLAN-CTRL = 3.3V
 - J1 = 2-3: 3.3V from uSD connector


Modules Settings
================
Jumper settings on AzureWave AW-NM191NF-uSD Module:
  - J11 1-2: VIO_SD 1.8V (Voltage level of SDIO pins is 1.8V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)


Board settings
==============
Make sure C28 is welded.
