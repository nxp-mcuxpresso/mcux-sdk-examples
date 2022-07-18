Overview
========
This example showcases the Azure device update (ADU) feature. It connects to Azure IoT Hub and starts interacting with the service,
Device Update for IoT Hubs. When the example is running, it will report the device status, and fetch the device update information.

In this example, the device credential will be stored in flash securely.

Prerequisites

Before running the example, need to set up a device in Azure IoT Hub and set up the Device Update for IoT Hubs service.

Here, we demonstrate how to setup a device in Azure IoT Hub. If you are not familiar with Azure CLI, please refer to
the document for the details. (https://docs.microsoft.com/en-us/azure/iot-hub/)

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

9. In the web page of Azure portal, create a new service, Device Update for IoT Hubs.
   Please refer to https://docs.microsoft.com/en-us/azure/iot-hub-device-update/create-device-update-account


The following steps are about board setup.

1. Install the bootloader, mcuboot_opensource, which is in the directory, boards/evkmimxrt1060/mcuboot_opensource/. 
   This bootloader will work with the azure_iot_embedded_sdk_adu example to implement the Azure Device Update feature. 
   Build the bootloader image and burn it into flash.

2. Install a python program, imgtool, which is used for processing the raw image in the following steps (These steps
   need to be done on a Linux PC.):

    > pip3 install imgtool==1.7.2

3. Build an initial image. Enter the ADU example directory, boards/evkmimxrt1060/azure_rtos_examples/azure_iot_embedded_sdk_adu/.
   Change the value of these macros in sample_config.h on demand:

    SAMPLE_DEVICE_MANUFACTURER
    SAMPLE_DEVICE_MODEL
    SAMPLE_UPDATE_ID_PROVIDER
    SAMPLE_UPDATE_ID_NAME
    SAMPLE_UPDATE_ID_VERSION

   Build the example and get a binary image named azure_iot_embedded_sdk_adu.bin. 

4. Sign the image and write it into the flash. Sign the image azure_iot_embedded_sdk_adu.bin created in the previous step,
   using the script gen_update_files.sh, for example:

    > ./gen_update_files.sh armgcc/flexspi_nor_release/azure_iot_embedded_sdk_adu.bin

   It will generate an image file, like MIMXRT1060.azure_iot_embedded_sdk_adu.1.0.0.signed.bin. Then, write the signed image
   into the flash at the address 0x60040000. The generated manifest file is no use this time. Note that this key
   ./keys/sign-rsa2048-priv.pem for signing images is from the mcuboot_opensource project. It can be changed on demand.

5. Build an update image which has a greater version than the initial image. First, increase the version number
   in the macro SAMPLE_UPDATE_ID_VERSION in sample_config.h, for example, changing from 1.0.0 to 1.1.0. Then,
   build the azure_iot_embedded_sdk_adu example again and get a new image for update.

6. To create an update image that can be imported in the Azure IoT hub, need to sign the update image and
   generate a manifest file for it. To do this, execute the script gen_update_files.sh.

    ./gen_update_files.sh armgcc/flexspi_nor_release/azure_iot_embedded_sdk_adu.bin

   It will generate files, like NXP.MIMXRT1060.1.1.0.importmanifest.json and MIMXRT1060.azure_iot_embedded_sdk_adu.1.1.0.signed.bin


The following steps are about how to deploy an update image.

1. After write the initial image into flash at the address 0x60040000, boot up the device. For the first time, it will prompt you
   to enter device credential you got from previous steps. When the serial outputs:

    Please input HostName:

   Input "{MyIoTHubName}.azure-devices.net" (not include quote mark)

    Please input DeviceId:

   Input "{MyDeviceId}" (not include quote mark)

    Please input SharedAccessKey:

   Input "{MySymmetricKey}" (not include quote mark)
   Make sure the device has connected to Azure IoT Hub successfully.

2. Create a device group. Open the created device in Azure IoT Hub. Add the tag ADUGroup in the Device twin page.
   For example, if the name of your device group is "adu_group", add it like:

    "tags": {
            "ADUGroup": "adu_group"
        },

   Note that do not forget to save. Please refer to
     https://docs.microsoft.com/en-us/azure/iot-hub-device-update/create-update-group

3. Import the update files. In the left panel of Azure IoT Hub, find "Updates" in "Device management" and click it.
   On the upper area of the page, you will see "Welcome to the new Device update (preview).". DO NOT switch to
   the older version. In the Updates tab page, click the button "Import a new update". Then, select a storage container,
   upload these local files, MIMXRT1060.azure_iot_embedded_sdk_adu.1.1.0.signed.bin and NXP.MIMXRT1060.1.1.0.importmanifest.json.
   This creates a new update entry in the Updates tab page in minutes.
   Please refer to https://docs.microsoft.com/en-us/azure/iot-hub-device-update/import-update

4. Deploy an update image. Enter the Groups and Deployments tag page and click the Add group button to add a new group, "adu_group".
   Click the "adu_group" group, then enter the Current deployment tab. On the right of the page, you will see a deploy/redeploy
   link if possible. Click it to start deployment.
   Please refer to https://docs.microsoft.com/en-us/azure/iot-hub-device-update/deploy-update
   Wait for a moment, the device will receive the update message, and print:

     Received new update: Provider: NXP; Name: MIMXRT1060, Version: 1.1.0

   The update process will start automatically. After writing the update image into flash, the device will reset to start up
   the new update image.

5. When the new update image starts up, the serial will output:

     Start the azure_iot_embedded_sdk_adu example (v1.1.0)


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



Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- EVK-MIMXRT1060 board
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
Bootloader Version 1.9.0
Primary image: magic=good, swap_type=0x2, copy_done=0x1, image_ok=0x1
Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
Boot source: none
Swap type: none
Bootloader chainload address offset: 0x40000
Reset_Handler address offset: 0x40400
Jumping to the image


Start the azure_iot_embedded_sdk_adu example (v1.0.0)
Did not find any device connection string.
Please input a device connection string.
Please input HostName: adu.azure-devices.net
Please input DeviceId: adu_test2
Please input SharedAccessKey: IZaLpAsssZXJzHtuAQ+s1vubKfQ6j4el9hFDtli18=

DHCP In Progress...
IP address: 192.168.0.16
Mask: 255.255.255.0
Gateway: 192.168.0.1
DNS Server address: 192.168.0.1
SNTP Time Sync...0.pool.ntp.org
SNTP Time Sync successfully.
[INFO] Azure IoT Security Module has been enabled, status=0
IoTHub Host Name: adu.azure-devices.net; Device ID: adu_test2.
Connected to IoTHub.
[INFO] ADU agent started successfully!
Device Properties: manufacturer: NXP, model: MIMXRT1060
Installed Update ID: provider: NXP, name: MIMXRT1060, version: 1.0.0
Sent properties request.
Telemetry message send: {"Message ID: ":0}.
[INFO] Cancel Command received
[INFO] Azure IoT Security Module message is empty
Telemetry message send: {"Message ID: ":1}.
Telemetry message send: {"Message ID: ":2}.
Telemetry message send: {"Message ID: ":3}.
Telemetry message send: {"Message ID: ":4}.
Telemetry message send: {"Message ID: ":5}.
Telemetry message send: {"Message ID: ":6}.
Telemetry message send: {"Message ID: ":7}.
Telemetry message send: {"Message ID: ":8}.
Telemetry message send: {"Message ID: ":9}.
Telemetry message send: {"Message ID: ":10}.
Telemetry message send: {"Message ID: ":11}.
Telemetry message send: {"Message ID: ":12}.
Telemetry message send: {"Message ID: ":13}.
Telemetry message send: {"Message ID: ":14}.
Telemetry message send: {"Message ID: ":15}.
Telemetry message send: {"Message ID: ":16}.
Telemetry message send: {"Message ID: ":17}.
Telemetry message send: {"Message ID: ":18}.
Telemetry message send: {"Message ID: ":19}.
Received new update: Provider: NXP; Name: MIMXRT1060, Version: 1.1.0
[INFO] Updating host firmware...
[INFO] Firmware downloading...
firmware size: 0x3c3e8
Erase the update partition (start: 0x240000, size: 0x200000)
[INFO] Getting download data... 5440
[INFO] Getting download data... 15520
[INFO] Getting download data... 15520
[INFO] Getting download data... 16320
[INFO] Getting download data... 27200
[INFO] Getting download data... 27200
[INFO] Getting download data... 29920
[INFO] Getting download data... 29920
[INFO] Getting download data... 31280
[INFO] Getting download data... 47664
[INFO] Getting download data... 57120
[INFO] Getting download data... 57120
[INFO] Getting download data... 59840
[INFO] Getting download data... 61200
[INFO] Getting download data... 62560
[INFO] Getting download data... 72144
[INFO] Getting download data... 87040
[INFO] Getting download data... 87040
[INFO] Getting download data... 88528
[INFO] Getting download data... 89760
[INFO] Getting download data... 92480
[INFO] Getting download data... 95200
[INFO] Getting download data... 111520
[INFO] Getting download data... 119680
[INFO] Getting download data... 121104
[INFO] Getting download data... 123760
[INFO] Getting download data... 123760
[INFO] Getting download data... 138720
[INFO] Getting download data... 145584
[INFO] Getting download data... 145584
[INFO] Getting download data... 149600
[INFO] Getting download data... 149600
[INFO] Getting download data... 152320
[INFO] Getting download data... 152320
[INFO] Getting download data... 153680
[INFO] Getting download data... 155040
[INFO] Getting download data... 156464
[INFO] Getting download data... 157888
[INFO] Getting download data... 164560
[INFO] Getting download data... 179520
[INFO] Getting download data... 179520
[INFO] Getting download data... 180944
[INFO] Getting download data... 187680
[INFO] Getting download data... 187680
[INFO] Getting download data... 189040
[INFO] Getting download data... 191760
[INFO] Getting download data... 193120
[INFO] Getting download data... 209440
[INFO] Getting download data... 209440
[INFO] Getting download data... 213520
[INFO] Getting download data... 213520
[INFO] Getting download data... 214880
[INFO] Getting download data... 216240
[INFO] Getting download data... 218960
[INFO] Getting download data... 218960
[INFO] Getting download data... 225760
[INFO] Getting download data... 235280
[INFO] Getting download data... 239360
[INFO] Getting download data... 239360
[INFO] Getting download data... 246160
[INFO] Firmware downloaded
[INFO] Firmware installing...
write magic number offset = 0x43ff00
[INFO] Firmware installed
[INFO] Firmware applying...
hello sbl.
Bootloader Version 1.9.0
Primary image: magic=good, swap_type=0x2, copy_done=0x1, image_ok=0x1
Secondary image: magic=good, swap_type=0x1, copy_done=0x3, image_ok=0x3
Boot source: none
Swap type: test
Starting swap using move algorithm.
erasing trailer; fa_id=1
initializing status; fa_id=1
writing swap_info; fa_id=1 off=0x1fffd8 (0x23ffd8), swap_type=0x2 image_num=0x0
writing swap_size; fa_id=1 off=0x1fffd0 (0x23ffd0)
writing magic; fa_id=1 off=0x1ffff0 (0x23fff0)
erasing trailer; fa_id=2
writing copy_done; fa_id=1 off=0x1fffe0 (0x23ffe0)
Bootloader chainload address offset: 0x40000
Reset_Handler address offset: 0x40400
Jumping to the image


Start the azure_iot_embedded_sdk_adu example (v1.1.0)
Get the device connection string from an encrypted file.
HostName: adu.azure-devices.net
DeviceId: adu_test2
SharedAccessKey: IZaLpAsssZXJzHtuAQ+s1vubKfQ6j4el9hFDtli18=

Do you want to update the device information? (Y/N) N

DHCP In Progress...
IP address: 192.168.0.16
Mask: 255.255.255.0
Gateway: 192.168.0.1
DNS Server address: 192.168.0.1
SNTP Time Sync...0.pool.ntp.org
SNTP Time Sync successfully.
[INFO] Azure IoT Security Module has been enabled, status=0
IoTHub Host Name: adu.azure-devices.net; Device ID: adu_test2.
Connected to IoTHub.
[INFO] ADU agent started successfully!
Device Properties: manufacturer: NXP, model: MIMXRT1060
Installed Update ID: provider: NXP, name: MIMXRT1060, version: 1.1.0
Sent properties request.
Telemetry message send: {"Message ID: ":0}.
[INFO] Azure IoT Security Module message is empty
Telemetry message send: {"Message ID: ":1}.
Telemetry message send: {"Message ID: ":2}.
Telemetry message send: {"Message ID: ":3}.

