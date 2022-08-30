eIQ inference with DeepView RT

Content
-------
1. Introduction
2. Directory structure
3. eIQ inference with DeepView RT Example
4. Documentation
5. Library configuration
6. Release notes
7. Limitations
8. Deepview RT modelrunner example overview

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

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                         - Example build projects
|         -- deepviewrt_modelrunner            - deepviewrt modelrunner example
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
The library is based on DeepView RT version 2.4.44.

7. Limitations
--------------
* DeepView RT library depends on NewLib for MCUXpresso IDE.

8. Deepview RT modelrunner overview
-----------------------------------

The lwip_dhcp demo application demonstrates a DHCP demo on the lwIP TCP/IP stack.
The application acts as a DHCP client and prints the status as it is progressing.
Once the interface is being bound to an IP address obtained from DHCP server, address information is printed.

8.1. Prepare the Demo
================
8.1.1  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
8.1.2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
8.1.3.  Insert the Ethernet Cable into the target board's RJ45 port and connect it to a router (or other DHCP server capable device).
8.1.4.  Download the program to the target board.
8.1.5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

8.2. Running the demo
================
When the demo runs, the log would be seen on the terminal like:

Initializing PHY...

************************************************
 DHCP example
************************************************
 DHCP state       : SELECTING
 DHCP state       : REQUESTING
 DHCP state       : CHECKING
 DHCP state       : BOUND

 IPv4 Address     : 192.168.0.4
 IPv4 Subnet mask : 255.255.255.0
 IPv4 Gateway     : 192.168.0.1

8.3 Refer to eIQ Toolkit User's Guide to validate or profiling the model
