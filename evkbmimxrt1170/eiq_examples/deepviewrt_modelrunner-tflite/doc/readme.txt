eIQ inference with TFLiteMicro 2.6.0

Content
-------
1. Introduction
2. Directory structure
3. eIQ inference with TFLite Example
4. Documentation
5. Release notes
6. ModelRunner for TFLite Example

1. Introduction
---------------
DeepView RT ModelRunner for TFLiteMirco is similar service as ModelRunner for TFLite, but it can only stream 
test data to modelrunner service with TFLite backend, it can return Image inferrence result after receive
test data. 
ModelRunner for TFLite can work with eIQ modeltool to profiling model.

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                         - Example build projects
|         -- deepviewrt_modelrunner-tflite       - deepviewrt example
|
|-- middleware
    -- eiq
       -- tensorflow-lite          - TFLiteMirco 2.6.0

3. eIQ Inference with DeepView RT example
-----------------------------------------
3.1 Introduction
    The package contains modelrunner example applications using
    the TFLiteMirco library. The build projects can be found in
    the /boards/<board>/eiq_examples/deepviewrt* folders.

3.2 Toolchains supported
    - MCUXpresso IDE
    - IAR Embedded Workbench for ARM
    - Keil uVision MDK
    - ArmGCC - GNU Tools ARM Embedded

3.3 Supported board and settings
    - evkmimxrt1170:
      No special settings are required.
    - evkbimxrt1050:
      No special settings are required.
    - evkmimxrt1060:
      No special settings are required.
    - evkbimxrt1064:
      No special settings are required.
    - evkmimxrt1160:
      No special settings are required.

4. Documentation
----------------
    https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

5. Release notes
----------------
The library is based on TFLiteMicro 2.6.0.

6. ModelRunner for TFLite Example
---------------------------------
6.1 Running the demo

The ModelRunner for TFLite will run as one http service and can get data input. 
It dump the 

*************************************************
               TFLite Modelrunner
*************************************************
Initializing PHY...
 DHCP state       : SELECTING
 DHCP state       : REQUESTING
 DHCP state       : BOUND

 IPv4 Address     : 10.193.20.16
 IPv4 Subnet mask : 255.255.255.0
 IPv4 Gateway     : 10.193.20.254

Initialized TFliteMicro modelrunner server at port 10818

6.2 Stream test data to ModelRunner for TFLite service
On Linux or Windows PC, prepare one image (jpg or png formate). Then use following
command to post data to ModelRunner for TFLite service on device. The json response show the
TFLite Version, inferrence time and label index. 

# curl -XPOST -H 'Content-Type: image/*' --data-binary "@/home/xiao/panda.jpg" 'http://10.193.20.66:10818/v1?run=1' | jq
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 51055  100    57  100 50998      4   4374  0:00:14  0:00:11  0:00:03    15
{
  "engine": "TFLiteMicro-2.6.0",
  "timing": 10566,
  "index": 389
}
