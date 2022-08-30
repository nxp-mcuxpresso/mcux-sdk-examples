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
loop 1: 
CONV_2D took 342000000 us
DEPTHWISE_CONV_2D took 164000000 us
CONV_2D took 103000000 us
CONV_2D took 356000000 us
DEPTHWISE_CONV_2D took 165000000 us
CONV_2D took 95000000 us
CONV_2D took 171000000 us
DEPTHWISE_CONV_2D took 228000000 us
CONV_2D took 142000000 us
ADD took 8000000 us
CONV_2D took 171000000 us
DEPTHWISE_CONV_2D took 68000000 us
CONV_2D took 48000000 us
CONV_2D took 74000000 us
DEPTHWISE_CONV_2D took 67000000 us
CONV_2D took 67000000 us
ADD took 3000000 us
CONV_2D took 71000000 us
DEPTHWISE_CONV_2D took 67000000 us
CONV_2D took 67000000 us
ADD took 3000000 us
CONV_2D took 71000000 us
DEPTHWISE_CONV_2D took 22000000 us
CONV_2D took 76000000 us
CONV_2D took 200000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 196000000 us
ADD took 1000000 us
CONV_2D took 198000000 us
DEPTHWISE_CONV_2D took 43000000 us
CONV_2D took 196000000 us
ADD took 1000000 us
CONV_2D took 199000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 196000000 us
ADD took 1000000 us
CONV_2D took 198000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 300000000 us
CONV_2D took 452000000 us
DEPTHWISE_CONV_2D took 80000000 us
CONV_2D took 457000000 us
ADD took 2000000 us
CONV_2D took 453000000 us
DEPTHWISE_CONV_2D took 82000000 us
CONV_2D took 458000000 us
ADD took 2000000 us
CONV_2D took 451000000 us
DEPTHWISE_CONV_2D took 22000000 us
CONV_2D took 190000000 us
CONV_2D took 310000000 us
DEPTHWISE_CONV_2D took 37000000 us
CONV_2D took 327000000 us
ADD took 1000000 us
CONV_2D took 311000000 us
DEPTHWISE_CONV_2D took 37000000 us
CONV_2D took 327000000 us
ADD took 1000000 us
CONV_2D took 311000000 us
DEPTHWISE_CONV_2D took 36000000 us
CONV_2D took 653000000 us
CONV_2D took 828000000 us
AVERAGE_POOL_2D took 10000000 us
CONV_2D took 57000000 us
RESHAPE took 0 us
SOFTMAX took 0 us
run ms: 10565.000000 index 905, score: 0.196136 
loop 2: 
CONV_2D took 342000000 us
DEPTHWISE_CONV_2D took 164000000 us
CONV_2D took 103000000 us
CONV_2D took 356000000 us
DEPTHWISE_CONV_2D took 165000000 us
CONV_2D took 95000000 us
CONV_2D took 171000000 us
DEPTHWISE_CONV_2D took 227000000 us
CONV_2D took 142000000 us
ADD took 8000000 us
CONV_2D took 171000000 us
DEPTHWISE_CONV_2D took 68000000 us
CONV_2D took 47000000 us
CONV_2D took 74000000 us
DEPTHWISE_CONV_2D took 67000000 us
CONV_2D took 67000000 us
ADD took 3000000 us
CONV_2D took 71000000 us
DEPTHWISE_CONV_2D took 67000000 us
CONV_2D took 67000000 us
ADD took 2000000 us
CONV_2D took 71000000 us
DEPTHWISE_CONV_2D took 22000000 us
CONV_2D took 76000000 us
CONV_2D took 200000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 196000000 us
ADD took 2000000 us
CONV_2D took 198000000 us
DEPTHWISE_CONV_2D took 43000000 us
CONV_2D took 196000000 us
ADD took 1000000 us
CONV_2D took 199000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 197000000 us
ADD took 1000000 us
CONV_2D took 199000000 us
DEPTHWISE_CONV_2D took 42000000 us
CONV_2D took 299000000 us
CONV_2D took 453000000 us
DEPTHWISE_CONV_2D took 81000000 us
CONV_2D took 457000000 us
ADD took 2000000 us
CONV_2D took 454000000 us
DEPTHWISE_CONV_2D took 82000000 us
CONV_2D took 458000000 us
ADD took 2000000 us
CONV_2D took 451000000 us
DEPTHWISE_CONV_2D took 22000000 us
CONV_2D took 190000000 us
CONV_2D took 310000000 us
DEPTHWISE_CONV_2D took 37000000 us
CONV_2D took 327000000 us
ADD took 1000000 us
CONV_2D took 311000000 us
DEPTHWISE_CONV_2D took 37000000 us
CONV_2D took 328000000 us
ADD took 1000000 us
CONV_2D took 311000000 us
DEPTHWISE_CONV_2D took 36000000 us
CONV_2D took 653000000 us
CONV_2D took 828000000 us
AVERAGE_POOL_2D took 9000000 us
CONV_2D took 57000000 us
RESHAPE took 0 us
SOFTMAX took 0 us
run ms: 10566.000000 index 905, score: 0.196136 
run ms: 10565.500000

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
