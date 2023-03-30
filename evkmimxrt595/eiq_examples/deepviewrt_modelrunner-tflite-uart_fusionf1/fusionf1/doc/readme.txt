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
DeepView RT ModelRunner Uart for TFLiteMirco is uart based service as ModelRunner for TFLite, but it can only stream 
test data to modelrunner service with TFLite backend.
ModelRunner Uart for TFLite can work with eIQ modeltool to profiling model.

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                                - Example build projects
|         -- deepviewrt_modelrunner-tflite-uart       - deepviewrt example
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
    - evkmimxrt595:
      No special settings are required.
    - evkbimxrt685:
      No special settings are required.

4. Documentation
----------------
    https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

5. Release notes
----------------
The library is based on TFLiteMicro 2.6.0.

6. ModelRunner Uart for TFLite Example
---------------------------------
6.1 Running the demo

 1. Download the program to the target board.
 2. Use the scripts scripts/main.py to launch the http service on x86 platform to handle the http requests and forward the data to the board via uart.
   #python3 main.py
     * Serving Flask app "main" (lazy loading)
     * Environment: production
       WARNING: This is a development server. Do not use it in a production deployment.
       Use a production WSGI server instead.
     * Debug mode: on
     * Running on http://0.0.0.0:10919/ (Press CTRL+C to quit)
     * Restarting with stat
     * Debugger is active!
     * Debugger PIN: 132-343-000
 3. Connect the usb cable to x86.

6.2 Stream test data to ModelRunner for TFLite service
On Linux or Windows PC, run the deepview-validator or eIQ Model Tool to do the performance test.
The uri should be http://localhost:10919/serial/<board-serial-id>
For linux, the serial id can be found in /dev/serial/by-id:
  #ls /dev/serial/by-id/
   usb-NXP_Semiconductors_LPC-LINK2_CMSIS-DAP_V5.224_GRAXBQHR-if01
For Windows, the serial id is com<x>

- DeepviewRT validator:
 #deepview-validator --input_names input --output_names MobilenetV1/Predictions/Reshape_1 --samples 1 --uri http://localhost:10919/serial/GRAXBQHR --reference mobilenet_v1_1.0_224_pb.npz mobilenet_v1_1.0_224_pb_uint8.tflite
 
- eIQ Model Tool:
 Add target with uri 'http://localhost:10919/serial/GRAXBQHR' and click the 'Tools->Profile Model' button to to profiling model.

