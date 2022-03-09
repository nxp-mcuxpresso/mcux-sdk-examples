Overview
========
This example demonstrates how to communicate with Azure IoT Hub through MQTT.

Prerequisites
Before running the example, need to set up a device in Azure IoT Hub, and write the device parameters in the example code.

Here, we demonstrate how to setup a device in Azure IoT Hub. If you are not familiar with Azure CLI, please refer to
the document for the details. (https://docs.microsoft.com/en-us/azure/iot-hub/)

Note that these steps assume you use the Azure IoT Hub for the first time.

1. Register an Azure account.

2. Install Azure CLI locally, or use Azure CLoud Shell.

3. Before using any CLI commands locally, you need to sign in:
     > az login

4. Add the Microsoft Azure IoT Extension for Azure CLI.
     > az extension add --name azure-iot

5: Create a new resource group which is a logical container into which Azure IoT Hub are deployed and managed.
   {MyResourceGroupName}: Name of the new resource group
   {MyResourceLocation}: Location, for example, westus. Select a location from: az account list-locations -o table.
     > az group create --name {MyResourceGroupName} --location {MyResourceLocation}

6: Create a new IoT Hub in the resource group.
   {MyResourceGroupName}: The name of the resource group you just created.
   {MyIoTHubName}: Name of the new IoT Hub. This name must be globally unique. If failed, please try another name.
     > az iot hub create --resource-group {MyResourceGroupName} --name {MyIoTHubName}

7: Create a new device identity in the Hub IoT.
   {MyIoTHubName}: Name of the IoT Hub just created
   {MyDeviceId}: ID of the new device
     > az iot hub device-identity create --hub-name {MyIoTHubName} --device-id {MyDeviceId}

8: Create a new device SAS token for the device {MyDeviceId}. Note that the token is only valid in 3600 seconds.
     > az iot hub generate-sas-token --hub-name {MyIoTHubName} --device-id {MyDeviceId}
   If you want to set a specified valid duration, please use the parameter, --duration {seconds}, to set the valid
   token duration in seconds.
   The command result is in the JSON format, like:
     {
        "sas": "{MySASToken}"
     }

9. Write the above device parameters into the source code, sample_azure_iot.c, in your project. Fill these three macros,
   HOST_NAME, DEVICE_ID, DEVICE_SAS.
     HOST_NAME: {MyIoTHubName}.azure-devices.net
     DEVICE_ID: {MyDeviceId}
     DEVICE_SAS: {MySASToken}
   For example:
     #define HOST_NAME "test-hub.azure-devices.net"
     #define DEVICE_ID "test-dev"
     #define DEVICE_SAS "SharedAccessSignature sr=test-hub.azure-devices.net%2Fdevices%2Ftest-dev&sig=57jVRiSOeoX9g4aI6iyP6tFzrjEdam5SpdNITeeUbVY%3D&se=1615181586"

10. Build the code and write it into the on-board Flash.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- EVKB-IMXRT1050 board
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
When the demo is running, the serial port will output, for example:

Start the azure_iot_mqtt example...
DHCP In Progress...
IP address: 10.193.20.67
Mask: 255.255.255.0
Gateway: 10.193.20.254
DNS Server address: 10.192.130.201
Connected to server
Subscribed to server
[Published] topic = devices/imxrt1050/messages/events/, message: {"temperature": 21}
[Published] topic = devices/imxrt1050/messages/events/, message: {"temperature": 22}
[Published] topic = devices/imxrt1050/messages/events/, message: {"temperature": 23}
[Published] topic = devices/imxrt1050/messages/events/, message: {"temperature": 24}

