DeepViewRT labelimage example

Content
-------
1. Introduction
2. Directory structure
3. eIQ inference with DeepView RT Example
4. Documentation
5. Library configuration
6. Release notes
7. Limitations
8. Labelimage example execution

1. Introduction
---------------
DeepView RT is an implements a neural network inference engine suitable for embedded processors,
graphics processors, and microcontrollers. The DeepView RT Model (RTM) format supports in-place
interpretation which allows it to be stored directly in flash and used as-is. This allows for 
resource constrained MCU (ex: Cortex-M) platforms as the actual model and its weights do not 
consume any RAM and instead can be kept in-place in flash memory. A small amount of memory is
required for the network evaluation graph as well as a buffer cache for storing the volatile 
input/output data at inference time, but no memory is required for the actual weights which can
remain in flash. If the cache is large enough to host the weights as well, they will be streamed
from flash on-demand as a performance optimization, and for maximum performance on parts with 
adequate memory the entire model can be stored in RAM.
DeepView RT ModelRunner is a dedicated service for hosting and evaluating RTM graphs through a
set of RPC protocols, it provides an HTTP REST API for loading an evaluating models. It can work
with eIQ toolkit to evaluating and profiling models. 
Note: DeepView RT supports only a subset of operators available in
      TensorFlow. The eIQ toolkit can convert Tensorflow 2.x model to RTM model, please refer to
      eIQ toolkit about model conversion. 

This "labelimage" example is a demonstration baremetal program which uses the SDRAM to allocate
the memory pool, and it integrates a user defined model(mobilenet_v1_0.25_160) and a user
provided image(JPG) to classify, the model is trained using tensorflow, then convert to binary
runtime model (RTM) with eIQ Portal.

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                         - Example build projects
|         -- deepviewrt_labelimage             - deepviewrt classification with static image example
|         -- deepviewrt_modelrunner            - deepviewrt modelrunner example
|         -- deepviewrt_modelrunner-glow       - deepviewrt with glow example
|
|-- middleware
    -- eiq
       -- deepviewrt
          -- include                   - DeepviewRT library header files
          -- lib                       - DeepviewRT library binaries

3. eIQ Inference with DeepView RT example
-----------------------------------------
3.1 Introduction
    The package contains modelrunner example applications using
    the DeepView RT library. The build projects can be found in
    the /boards/<board>/eiq_examples/deepviewrt* folders.

3.2 Toolchains supported
    - MCUXpresso IDE
    - IAR Embedded Workbench for ARM
    - Keil uVision MDK
    - ArmGCC - GNU Tools ARM Embedded

3.3 Supported board and settings
    - evkmimxrt1170:
      No special settings are required.
    - evkmimxrt1064:
      No special settings are required.
    - evkmimxrt1060:
      No special settings are required.
    - evkbimxrt1050:
      No special settings are required.
    - evkmimxrt1160:
      No special settings are required.

4. Documentation
----------------
    https://www.nxp.com/design/software/development-software/eiq-ml-development-environment:EIQ

5. Library configuration
------------------------
5.1. Stack memory configuration
     During the library compilation, based on the stack memory configuration,
     the EIGEN_STACK_ALLOCATION_LIMIT macro definition can be set to the maximum
     size of temporary objects that can be allocated on the stack
     (they will be dynamically allocated instead). A high number may cause stack
     overflow. A low number may decrease object allocation performance.

6. Release notes
----------------
The library is based on DeepView RT version 2.4.36.

7. Limitations
--------------
* Labelimage example is not supported by IAR toolchain currently.

8. Labelimage example execution
------------------------------

Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert the Ethernet Cable into the target board's RJ45 port and connect it to a router (or other DHCP server capable device).
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs, the log and inference result would be seen on the terminal as below.

   Result: giant panda (85%) -- decode: 52 ms runtime: 672 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 442 ms
   Result: giant panda (85%) -- decode: 52 ms runtime: 442 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 443 ms
   Result: giant panda (85%) -- decode: 51 ms runtime: 442 ms
