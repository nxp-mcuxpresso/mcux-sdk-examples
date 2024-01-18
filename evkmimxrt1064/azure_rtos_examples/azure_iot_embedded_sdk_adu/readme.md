Overview
========
This example showcases the Azure device update (ADU) feature. It connects to Azure IoT Hub and starts interacting with the service,
Device Update for IoT Hubs. When the example is running, it will report the device status, and fetch the device update information.

In this example, the device credential will be stored in flash securely.

Prerequisites

Azure IoT Hub Setup

  Before running the example, need to set up a device in Azure IoT Hub and set up the Device Update for IoT Hubs service.
  
  Here, we demonstrate how to setup a device in Azure IoT Hub. If you are not familiar with Azure CLI, please refer to
  the document for the details. (https://learn.microsoft.com/en-us/azure/iot-hub/)
  
  Note that these steps assume you use the Azure IoT Hub for the first time. The file directory is relative to the root
  directory of MCU SDK package.
  
    1. Register an Azure account.
    
    2. Install Azure CLI locally, or use Azure Cloud Shell.
    
    3. Before using any CLI commands locally, you need to sign in:
         > az login
    
    4. Add the Microsoft Azure IoT Extension for Azure CLI.
         > az extension add --name azure-iot
    
    5. Create a new resource group which is a logical container into which Azure IoT Hub are deployed and managed.
       {MyResourceGroupName}: Name of the new resource group
       {MyResourceLocation}: Location, for example, westus. Select a location from: az account list-locations -o table.
         > az group create --name {MyResourceGroupName} --location {MyResourceLocation}
    
    6. Create a new IoT Hub in the resource group.
       {MyResourceGroupName}: The name of the resource group you just created.
       {MyIoTHubName}: Name of the new IoT Hub. This name must be globally unique. If failed, please try another name.
         > az iot hub create --resource-group {MyResourceGroupName} --name {MyIoTHubName}
    
    7. Create a new device identity in the Hub IoT.
       {MyIoTHubName}: Name of the IoT Hub just created
       {MyDeviceId}: ID of the new device
         > az iot hub device-identity create --hub-name {MyIoTHubName} --device-id {MyDeviceId}
    
    8. Get the primary key of the device.
         > az iot hub device-identity show --hub-name {MyIoTHubName} --device-id {MyDeviceId}
       Find the primaryKey in the command result. It's the primary symmetric key, {MySymmetricKey}. Like:
         "authentication": {
             "symmetricKey": {
                 "primaryKey": {MySymmetricKey},
    
    9. Add the tag ADUGroup in the Device twin page. For example, if the name of your device group is "adu_group", add it like:
    
        "tags": {
                "ADUGroup": "adu_group"
            },
    
       Note that do not forget to save. Please refer to
         https://learn.microsoft.com/en-us/azure/iot-hub-device-update/create-update-group
  
    10. In the web page of Azure portal, create a new service, Device Update for IoT Hubs.
        Please refer to https://learn.microsoft.com/en-us/azure/iot-hub-device-update/create-device-update-account
    

Local Board Setup

  1. Install the bootloader, mcuboot_opensource, which is in the SDK directory, boards/evkmimxrt1060/mcuboot_opensource/.
     This bootloader will work with the azure_iot_embedded_sdk_adu example to implement the Azure Device Update feature. 
     Build the bootloader image and burn it into flash.
  
  2. Install a python program, imgtool, which is used for processing the raw image in the following steps:
  
      > pip3 install imgtool==1.9.0
  
  3. Build an initial image. Enter the ADU example directory, boards/evkmimxrt1060/azure_rtos_examples/azure_iot_embedded_sdk_adu/.
     Change the value of these macros in sample_config.h on demand:
  
      SAMPLE_DEVICE_MANUFACTURER
      SAMPLE_DEVICE_MODEL
      SAMPLE_DEVICE_FIRMWARE_VERSION
  
     Note that the content of SAMPLE_UPDATE_ID_PROVIDER and SAMPLE_UPDATE_ID_NAME must only contain these ASCII characters:
       a-z, A-Z, 0-9, dot(.), dash(-). The size of the content is less than or equal to 64.
  
     Build the example and get a binary image named azure_iot_embedded_sdk_adu.bin. 
  
  4. Sign the image and write it into the flash. Sign the image azure_iot_embedded_sdk_adu.bin created in the previous step,
     using the script sign_image.py in the scripts directory to sign the image and add an image header, for example:
  
     > cd scripts
     > python sign_image.py -k ../keys/sign-rsa2048-priv.pem -v "1.0.0" -i ../armgcc/flexspi_nor_release/azure_iot_embedded_sdk_adu.bin -f
  
     Note the -f parameter, it means generating an image for the first boot. It will generate an flash image file, like
     azure_iot_embedded_sdk_adu.1.0.0.signed.1667447086.boot.bin. The image size is 2097152 (2M) by default, which is the same as the size
     of one flash partition. If it's not, specify the flash partition size in the command line. For example, the flash partition size
     for evkmimxrt1064 is 1572864 (1.5M), so the command is like:

     > python sign_image.py -k ../keys/sign-rsa2048-priv.pem -v "1.0.0" -i ../armgcc/flexspi_nor_release/azure_iot_embedded_sdk_adu.bin -f -s 1572864

     The flash partition size is calculated by BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP. (these macros are defined in mcuboot/flash_partitioning.h)

     Then, write the signed image into the flash at the address of the first partition. The flash address depends on a specific board. It is
     defined in the macro BOOT_FLASH_ACT_APP in mcuboot/flash_partitioning.h. For example, it's 0x60040000 on evkmimxrt1060.

     Note that this key, ./keys/sign-rsa2048-priv.pem, for signing images is the same as the one in the mcuboot_opensource project.
     It's just for test. 
  
  5. After write the initial image into flash, boot up the device. For the first time, it will prompt you to enter device credential
     you got from Azure IoT hub. When the serial outputs:
  
      Please input HostName:
  
     Input "{MyIoTHubName}.azure-devices.net" (not include quote mark)
  
      Please input DeviceId:
  
     Input "{MyDeviceId}" (not include quote mark)
  
      Please input SharedAccessKey:
  
     Input "{MySymmetricKey}" (not include quote mark)

     Make sure the device can connect to Azure IoT Hub successfully. If failed, please check if NTP is successful,
     if the Azure IoT hub server can be accessed in your network.
  

Device Update Process

  1. Build an update image which has a greater version than the initial image. First, increase the version number
     in the macro SAMPLE_DEVICE_FIRMWARE_VERSION in sample_config.h, for example, changing from 1.0.0 to 1.1.0. Then,
     build the azure_iot_embedded_sdk_adu example again.
  
  2. To create an update image which can be imported in the Azure IoT hub, need to sign the update image and generate
     a manifest file for it. To do this, first execute the script sign_image.py.
  
      > cd scripts
      > python sign_image.py -k ../keys/sign-rsa2048-priv.pem -v "1.1.0" -i ../armgcc/flexspi_nor_release/azure_iot_embedded_sdk_adu.bin
  
     It will generate a signed image, like azure_iot_embedded_sdk_adu.1.1.0.signed.1667547086.bin.
  
  3. Generate a manifest file. If the macros in sample_config.h have been changed, please change the constants
     in the header of the script gen_manifest.py accordinlgy. Then, input this command:
  
      > python gen_manifest.py -v "1.1.0" -i azure_iot_embedded_sdk_adu.1.1.0.signed.1667547086.bin
  
  4. Import the update files including a signed image and a manifest json file. In the left panel of Azure IoT Hub, find "Updates"
     in "Device management" and click it. In the Updates tab page, click the button "Import a new update". Then, select a storage container,
     upload these files, one image file and one json file. This creates a new update entry in the Updates tab page.
     Please refer to https://docs.microsoft.com/en-us/azure/iot-hub-device-update/import-update
  
  5. Deploy an update image. Enter the "Groups and Deployments" tab page and click the group name which is the same as the tag value of ADUGroup.
     In the Group details page, enter the Current updates tab. On the right side of the page, you will see a deploy/redeploy link beside the version
     you just imported. Click it to start deployment. Please refer to https://docs.microsoft.com/en-us/azure/iot-hub-device-update/deploy-update.
     Wait for a moment, the device will receive the update message, and print, for example:
  
       Received new update: Provider: NXP; Name: MIMXRT1060, Version: 1.1.0
  
     The update process will start automatically. After writing the update image into flash, we can reset the device to start up the updated image.


Time sync problem

  If an error message as below appears in the serial output, it's because the board cannot connect to the NTP server.
  It may cause device authentication failure.
  
      SNTP Time Sync failed.
  
  To resolve it, one method is to replace SAMPLE_SNTP_SERVER_NAME in the main.c file with an available NTP server.
  Another method just for test is to update the default Epoch time in the SAMPLE_SYSTEM_TIME macro in the main.c file.
  
  On Linux, use the command to get the current Epoch time, for example:
  
      $ date +%s
      1610343882
  
  On Windows 10, use the following command in PowerShell to get the current Epoch time, for example:
  
      PS C:\Users> (New-TimeSpan -Start (Get-Date 01/01/1970) -End (Get-Date)).TotalSeconds
      1610343882.02736
  
  Update the value of SAMPLE_SYSTEM_TIME to the current Epoch time, for example, 1610343882. Then, rebuild the project.



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
- Network cable RJ45 standard
- EVK-MIMXRT1064 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert Cable to Ethernet RJ45 port and connect it to a ethernet switch.
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.

Running the demo
================
When the device update is running, the serial port will output, for example:


hello sbl.
Disabling flash remapping function
Bootloader Version 1.9.0
Primary   slot: version=1.0.0+0
Secondary slot: Image not found
Image 0 loaded from the primary slot
Bootloader chainload address offset: 0x40000
Reset_Handler address offset: 0x40400
Jumping to the image


Booting the primary slot - flash remapping is disabled

Start the azure_iot_embedded_sdk_adu example (1.0.0)
Get device credential from macros.

HostName: adu.azure-devices.net
DeviceId: adu_test2
SharedAccessKey: IZaLpAsssZXJzHtuAQ+s1vubKfQ6j4el9hFDtli18=

Do you want to update the device credential? (Y/N) N

MAC address: 00:11:22:33:44:56
DHCP In Progress...
IP address: 10.193.20.181
Mask: 255.255.255.0
Gateway: 10.193.20.254
DNS Server address: 165.114.89.4
SNTP Time Sync...0.pool.ntp.org
SNTP Time Sync successfully.
[INFO] Azure IoT Security Module has been enabled, status=0
IoTHub Host Name: adu.azure-devices.net; Device ID: adu_test2.
Connected to IoTHub.
[INFO] ADU agent started successfully!
Device Properties: manufacturer: NXP, model: MIMXRT1xxx, installed criteria: 1.0.0.
Sent properties request.
Telemetry message send: {"temperature":22}.
[INFO] Cancel Command received
Received all properties
[INFO] Azure IoT Security Module message is empty
Telemetry message send: {"temperature":22}.
Telemetry message send: {"temperature":22}.
Telemetry message send: {"temperature":22}.
Received new update: Provider: NXP; Name: MIMXRT1xxx, Version: 1.1.0
Received writable property
[INFO] Updating firmware...
[INFO] Manufacturer: NXP
[INFO] Model: MIMXRT1xxx
[INFO] Firmware downloading...
firmware size: 0x3e318
Erase the update partition (start: 0x240000, size: 0x200000)
Telemetry message send: {"temperature":22}.
[INFO] Getting download data... 539
[INFO] Getting download data... 1899
[INFO] Getting download data... 3259
[INFO] Getting download data... 4619
[INFO] Getting download data... 5979
[INFO] Getting download data... 7339
[INFO] Getting download data... 8699
[INFO] Getting download data... 10059
[INFO] Getting download data... 11419
[INFO] Getting download data... 12779
[INFO] Getting download data... 14139
[INFO] Getting download data... 15499
[INFO] Getting download data... 16859
[INFO] Getting download data... 16923
[INFO] Getting download data... 18219
[INFO] Getting download data... 19579
[INFO] Getting download data... 20939
[INFO] Getting download data... 22299
[INFO] Getting download data... 23659
[INFO] Getting download data... 25019
[INFO] Getting download data... 26379
[INFO] Getting download data... 27739
[INFO] Getting download data... 29099
[INFO] Getting download data... 30459
[INFO] Getting download data... 31819
[INFO] Getting download data... 33179
[INFO] Getting download data... 34539
[INFO] Getting download data... 35899
[INFO] Getting download data... 37259
[INFO] Getting download data... 38619
[INFO] Getting download data... 39979
[INFO] Getting download data... 41339
[INFO] Getting download data... 42699
[INFO] Getting download data... 44059
[INFO] Getting download data... 45419
[INFO] Getting download data... 46779
[INFO] Getting download data... 48139
[INFO] Getting download data... 49499
[INFO] Getting download data... 50859
[INFO] Getting download data... 52219
[INFO] Getting download data... 53579
[INFO] Getting download data... 54939
[INFO] Getting download data... 56299
[INFO] Getting download data... 57659
[INFO] Getting download data... 57723
[INFO] Getting download data... 59019
[INFO] Getting download data... 60379
[INFO] Getting download data... 61739
[INFO] Getting download data... 63099
[INFO] Getting download data... 64459
[INFO] Getting download data... 65819
[INFO] Getting download data... 67179
[INFO] Getting download data... 68539
[INFO] Getting download data... 69899
[INFO] Getting download data... 71259
[INFO] Getting download data... 72619
[INFO] Getting download data... 73979
[INFO] Getting download data... 74043
[INFO] Getting download data... 75339
[INFO] Getting download data... 76699
[INFO] Getting download data... 78059
[INFO] Getting download data... 79419
[INFO] Getting download data... 80779
[INFO] Getting download data... 82139
[INFO] Getting download data... 83499
[INFO] Getting download data... 84859
[INFO] Getting download data... 84923
[INFO] Getting download data... 86283
[INFO] Getting download data... 87643
[INFO] Getting download data... 89003
[INFO] Getting download data... 90363
[INFO] Getting download data... 91659
[INFO] Getting download data... 93019
[INFO] Getting download data... 94379
[INFO] Getting download data... 95739
[INFO] Getting download data... 97099
[INFO] Getting download data... 98459
[INFO] Getting download data... 99819
[INFO] Getting download data... 101179
[INFO] Getting download data... 102539
[INFO] Getting download data... 103899
[INFO] Getting download data... 105259
[INFO] Getting download data... 106619
[INFO] Getting download data... 107979
[INFO] Getting download data... 109339
[INFO] Getting download data... 110699
[INFO] Getting download data... 112059
[INFO] Getting download data... 113419
[INFO] Getting download data... 114779
[INFO] Getting download data... 116139
[INFO] Getting download data... 117499
[INFO] Getting download data... 118859
[INFO] Getting download data... 120219
[INFO] Getting download data... 120283
[INFO] Getting download data... 121643
[INFO] Getting download data... 123003
[INFO] Getting download data... 124299
[INFO] Getting download data... 125659
[INFO] Getting download data... 127019
[INFO] Getting download data... 128379
[INFO] Getting download data... 129739
[INFO] Getting download data... 131099
[INFO] Getting download data... 131163
[INFO] Getting download data... 132459
[INFO] Getting download data... 133819
[INFO] Getting download data... 135179
[INFO] Getting download data... 136539
[INFO] Getting download data... 137899
[INFO] Getting download data... 139259
[INFO] Getting download data... 140619
[INFO] Getting download data... 141979
[INFO] Getting download data... 142043
[INFO] Getting download data... 143403
[INFO] Getting download data... 144763
[INFO] Getting download data... 146123
[INFO] Getting download data... 147483
[INFO] Getting download data... 148779
[INFO] Getting download data... 150139
[INFO] Getting download data... 151499
[INFO] Getting download data... 152859
[INFO] Getting download data... 154219
[INFO] Getting download data... 155579
[INFO] Getting download data... 156939
[INFO] Getting download data... 158299
[INFO] Getting download data... 159659
[INFO] Getting download data... 161019
[INFO] Getting download data... 162379
[INFO] Getting download data... 163739
[INFO] Getting download data... 163867
[INFO] Getting download data... 165099
[INFO] Getting download data... 166459
[INFO] Getting download data... 167819
[INFO] Getting download data... 169179
[INFO] Getting download data... 170539
[INFO] Getting download data... 171899
[INFO] Getting download data... 171963
[INFO] Getting download data... 173259
[INFO] Getting download data... 174619
[INFO] Getting download data... 175979
[INFO] Getting download data... 177339
[INFO] Getting download data... 178699
[INFO] Getting download data... 180059
[INFO] Getting download data... 181419
[INFO] Getting download data... 182779
[INFO] Getting download data... 184139
[INFO] Getting download data... 185499
[INFO] Getting download data... 186859
[INFO] Getting download data... 188219
[INFO] Getting download data... 189579
[INFO] Getting download data... 190939
[INFO] Getting download data... 192299
[INFO] Getting download data... 193659
[INFO] Getting download data... 195019
[INFO] Getting download data... 196379
[INFO] Getting download data... 197739
[INFO] Getting download data... 199099
[INFO] Getting download data... 200459
[INFO] Getting download data... 201819
[INFO] Getting download data... 203179
[INFO] Getting download data... 204539
[INFO] Getting download data... 205899
[INFO] Getting download data... 207259
[INFO] Getting download data... 208619
[INFO] Getting download data... 209979
[INFO] Getting download data... 211339
[INFO] Getting download data... 212699
[INFO] Getting download data... 214059
[INFO] Getting download data... 215419
[INFO] Getting download data... 216779
[INFO] Getting download data... 218139
[INFO] Getting download data... 219499
[INFO] Getting download data... 220859
[INFO] Getting download data... 220923
[INFO] Getting download data... 222219
[INFO] Getting download data... 223579
[INFO] Getting download data... 224939
[INFO] Getting download data... 226299
[INFO] Getting download data... 227659
[INFO] Getting download data... 229019
[INFO] Getting download data... 230379
[INFO] Getting download data... 231739
[INFO] Getting download data... 233099
[INFO] Getting download data... 234459
[INFO] Getting download data... 234523
[INFO] Getting download data... 235883
[INFO] Getting download data... 237243
[INFO] Getting download data... 238539
[INFO] Getting download data... 239899
[INFO] Getting download data... 241259
[INFO] Getting download data... 242619
[INFO] Getting download data... 243979
[INFO] Getting download data... 245339
[INFO] Getting download data... 245403
[INFO] Getting download data... 246699
[INFO] Getting download data... 248059
[INFO] Getting download data... 249419
[INFO] Getting download data... 250779
[INFO] Getting download data... 252139
[INFO] Getting download data... 253499
[INFO] Getting download data... 254744
[INFO] Firmware downloaded
[INFO] Firmware installing...
write magic number offset = 0x23ff00
[INFO] Firmware installed
[INFO] Applying firmware...
[INFO] Manufacturer: NXP
[INFO] Model: MIMXRT1xxx
[INFO] Firmware applying...
[INFO] Firmware applied


Telemetry message send: {"temperature":22}.
Telemetry message send: {"temperature":22}.
Telemetry message send: {"temperature":22}.

