Overview
========
This example showcases the usability of NETX DUO API to connect to Azure IoT Hub and start interacting with Azure IoT services.
When the example is running, it will periodically report the temperature value to the IoT Hub.

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

8: Get the primary key of the device.
     > az iot hub device-identity show --hub-name {MyIoTHubName} --device-id {MyDeviceId}
   Find the primaryKey in the command result. It's the primary symmetric key, {MySymmetricKey}. Like:
     "authentication": {
         "symmetricKey": {
             "primaryKey": {MySymmetricKey},

9. Write the above device parameters into the source code, sample_config.h, in your project. Fill these three macros,
   HOST_NAME, DEVICE_ID, DEVICE_SAS.
     HOST_NAME: {MyIoTHubName}.azure-devices.net
     DEVICE_ID: {MyDeviceId}
     DEVICE_SYMMETRIC_KEY: {MySymmetricKey}
   For example:
     #define HOST_NAME "test-hub.azure-devices.net"
     #define DEVICE_ID "test-dev"
     #define DEVICE_SYMMETRIC_KEY "d/UdrshSDtn+WtcCHlaZyDcqIlUj5FpN8xqewCp2XYk="

10. Build the code and write it into the on-board Flash.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- EVKB-MIMXRT1060 board
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

Start the azure_iot_embedded_sdk example...
MAC address: 00:11:22:33:44:56
DHCP In Progress...
IP address: 192.168.0.8
Mask: 255.255.255.0
Gateway: 192.168.0.1
DNS Server address: 192.168.0.1
SNTP Time Sync...
SNTP Time Sync...
SNTP Time Sync...
SNTP Time Sync successfully.
[INFO] Azure IoT Security Module has been enabled, status=0
IoTHub Host Name: imxrthub.azure-devices.net; Device ID: MyCDevice.
Connected to IoTHub.
Telemetry message send: {"Message ID":0}.
Receive twin properties: {"desired": {"$version":1} ,"reported":{"$version":1}}
Telemetry message send: {"Message ID":1}.
Telemetry message send: {"Message ID":2}.
Telemetry message send: {"Message ID":3}.
Telemetry message send: {"Message ID":4}.
Telemetry message send: {"Message ID":5}.
Telemetry message send: {"Message ID":6}.
Telemetry message send: {"Message ID":7}.
Telemetry message send: {"Message ID":8}.
Telemetry message send: {"Message ID":9}.
Telemetry message send: {"Message ID":10}.

To read telemetry message from Azure IoT Hub, execute the command:

> az iot hub monitor-events --hub-name {MyIoTHubName} --device-id {MyDeviceId}

To read the device twin definition:

> az iot hub device-twin show --hub-name {MyIoTHubName} --device-id {MyDeviceId}

To invoke direct method on the device from the cloud: (note that the device just outputs the method detail)

> az iot hub invoke-device-method --hub-name {MyIoTHubName} --device-id {MyDeviceId} --method-name reboot --method-payload '{"time":"now"}'

Result
1. If the serial port outputs a message which is similar to the following message, it confirms that the Azure Device Twin function is OK.

   Receive twin properties :{"desired":{"GridPowerLimit2":31,"TariffCost":1.28,"$version":62},"reported":{"GridPowerLimit2":33,"TariffCost":2.25,"sample_report":"OK","$version":98}}

2. If an error message as below appears in the serial output, it's because the board cannot connect to the NTP server. To resolve it, replace SAMPLE_SNTP_SERVER_NAME with an available NTP server in the main.c file.

   SNTP Time Sync failed.

3. If there is no NTP server available, just for testing, the default Epoch time in the SAMPLE_SYSTEM_TIME macro can be updated manually
   in the main.c file. In Linux, use the command to get the current Epoch time, for example:

     $ date +%s
     1610343882

   In Windows 10, use the following command in PowerShell to get the current Epoch time, for example:

     PS C:\Users> (New-TimeSpan -Start (Get-Date 01/01/1970) -End (Get-Date)).TotalSeconds
     1610343882.02736

   Update the value of SAMPLE_SYSTEM_TIME to the current Epoch time, for example, 1610343882. Then, rebuild the project and test it.

