Supported Wi-Fi/BT/BLE modules
==============================
  - AzureWave AW-NM191NF-uSD
  - AzureWave AW-AM510-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2DS M.2 Module (EAR00386)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2EL M.2 Module (EAR00409)


Murata Solution Board settings
==============================
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 2DS module datasheet: https://www.embeddedartists.com/doc/ds/2DS_M2_Datasheet.pdf
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

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


Board settings
==============
uSD connector:
  - remove resistors: R368, R376, R347, R349, R365, R363 to disconnect the M.2 connector (decrease parasitic capacitance).

M.2 connector:
  - remove resistors: R364, R366, R351, R348, R377, R369 to disconnect the uSD connector (decrease parasitic capacitance).
  - populate R345 with 0Ohm resistor

Connect external power supply to J45, set J40: 1-2 and turn on SW6.
