Overview
========
This example showcases the usability of NETX DUO API to connect to Azure IoT Hub and start interacting with Azure IoT services.
When the example is running, it will periodically report the temperature value to the IoT Hub.

This example also used the Azure IoT Plug and Play feature. And, user can send the target temperature value to the device via the IoT Hub.

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

11. Install Azure IoT explorer. Refer to https://docs.microsoft.com/en-us/azure/iot-pnp/howto-use-iot-explorer#install-azure-iot-explorer
    The Azure IoT explorer is a graphical tool for interacting with and testing your IoT Plug and Play devices.
    Download link: https://github.com/Azure/azure-iot-explorer/releases

12. Use Azure IoT explorer. Refer to https://docs.microsoft.com/en-us/azure/iot-pnp/howto-use-iot-explorer#use-azure-iot-explorer
   a) Download the model file:
      1) Create a folder called 'models' on your local machine
      2) Download https://raw.githubusercontent.com/Azure/opendigitaltwins-dtdl/master/DTDL/v2/samples/Thermostat.json and save the JSON file to the 'models' folder.
   b) Connect to Azure IoT Hub:
      1) Retrieve your IoT Hub Connection String using the Azure CLI.
         > az iot hub connection-string show --hub-name {MyIoTHubName}
      2) The first time you run the tool, you're prompted for the IoT hub connection string.
   c) Configure the tool to use the model files you downloaded previously:
      1) From the home page in the tool, select IoT Plug and Play Settings, then + Add > Local folder.
      2) Select the models folder you created previously.
      3) Then select Save to save the settings.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- A Micro USB cable
- An Ethernet cable
- MIMXRT1170-EVKB board
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
3.  This example uses 1G port by default. If using the 100M ENET port, set the macro
    BOARD_NETWORK_USE_100M_ENET_PORT in board.h to 1.
4.  Compile the demo.
5.  Download the program to the target board.
6.  Insert an Ethernet cable to the default Ethernet RJ45 port labelled "1G ENET" and connect it to
    an Ethernet switch. Note that if set BOARD_NETWORK_USE_100M_ENET_PORT, the RJ45 port should be
    the one labelled "100M ENET".
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo is running, the serial port will output, for example:

Start the pnp_temperature_controller example...
MAC address: 00:11:22:33:44:56
DHCP In Progress...
IP address: 192.168.31.99
Mask: 255.255.255.0
Gateway: 192.168.31.1
DNS Server address: 192.168.31.1
SNTP Time Sync...
SNTP Time Sync...
SNTP Time Sync...
SNTP Time Sync successfully.
[INFO] Azure IoT Security Module has been enabled, status=0
Connected to IoTHub.
Temp Controller Telemetry message send: {"workingSet":1259}.
Thermostat thermostat1 Telemetry message send: {"temperature":22}.
Thermostat thermostat2 Telemetry message send: {"temperature":22}.
Received twin properties: {"desired":{"status":"OK","targetTemperature":30,"$version":4},"reported":{"sample_report":"OK","maxTempSinceLastReboot":30,"targetTemperature":{"value":30,"ac":200,"av":4,"ad":"success"},"serialNumber":"serial-no-123-abc","deviceInformation":{"__t":"c","manufacturer":"Sample-Manufacturer","model":"pnp-sample-Model-123","swVersion":"1.0.0.0","osName":"AzureRTOS","processorArchitecture":"Contoso-Arch-64bit","processorManufacturer":"Processor Manufacturer(TM)","totalStorage":1024,"totalMemory":128},"thermostat1":{"__t":"c","maxTempSinceLastReboot":22},"thermostat2":{"__t":"c","maxTempSinceLastReboot":22},"$version":14}}
Property=status arrived for Control component itself.  This does not support                writeable properties on it (all properties are on subcomponents)Property=targetTemperature arrived for Control component itself.  This does not support                writeable properties on it (all properties are on subcomponents)Temp Controller Telemetry message send: {"workingSet":1387}.
Thermostat thermostat1 Telemetry message send: {"temperature":22}.
Thermostat thermostat2 Telemetry message send: {"temperature":22}.
Temp Controller Telemetry message send: {"workingSet":1170}.
Thermostat thermostat1 Telemetry message send: {"temperature":22}.
Thermostat thermostat2 Telemetry message send: {"temperature":22}.
Temp Controller Telemetry message send: {"workingSet":1218}.
Thermostat thermostat1 Telemetry message send: {"temperature":22}.
Thermostat thermostat2 Telemetry message send: {"temperature":22}.

To read telemetry message from Azure IoT Hub, execute the command:

> az iot hub monitor-events --hub-name {MyIoTHubName} --device-id {MyDeviceId}

To read the device twin definition:

> az iot hub device-twin show --hub-name {MyIoTHubName} --device-id {MyDeviceId}

To invoke direct method on the device from the cloud:

> az iot hub invoke-device-method --hub-name {MyIoTHubName} --device-id {MyDeviceId} --method-name getMaxMinReport

Result
1. If the serial port outputs a message which is similar to the following message, it confirms that the Azure Device Twin function is OK.

   Received twin properties: {"desired":{"status":"OK","targetTemperature":30,"$version":4},"reported":{"sample_report":"OK","maxTempSinceLastReboot":30,"targetTemperature":{"value":30,"ac":200,"av":4,"ad":"success"},"serialNumber":"serial-no-123-abc","deviceInformation":{"__t":"c","manufacturer":"Sample-Manufacturer","model":"pnp-sample-Model-123","swVersion":"1.0.0.0","osName":"AzureRTOS","processorArchitecture":"Contoso-Arch-64bit","processorManufacturer":"Processor Manufacturer(TM)","totalStorage":1024,"totalMemory":128},"thermostat1":{"__t":"c","maxTempSinceLastReboot":22},"thermostat2":{"__t":"c","maxTempSinceLastReboot":22},"$version":14}}

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

