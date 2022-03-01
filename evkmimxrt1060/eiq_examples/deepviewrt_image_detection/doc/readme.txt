DeepViewRT image detection example

Content
-------
1. Introduction
2. Directory structure
3. eIQ inference with DeepView RT Example
4. Documentation
5. Library configuration
6. Release notes
7. Limitations
8. Application execution

1. Introduction
---------------
This "Image Detection" example shows the demonstration of using DeepViewRT API to do image detection
on an IMXRT platform. The application could identify multiple objects in a single image and outputs
the result via UART console. The result is including detected object name and bounding box array.

2. Directory structure
--------------------------------------
<MCUXpresso-SDK-root>
|-- boards
|   -- <board>
|      -- eiq_examples                         - Example build projects
|         -- deepviewrt_image_detection        - Identify multiple objects in a single image.
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
DeepView RT supports only a subset of operators available in TensorFlow.
The eIQ toolkit can convert Tensorflow model to RTM model, please refer to
eIQ toolkit about model conversion.

7. Limitations
--------------
* N/A

8. Application execution
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
The log below shows the output of the demo in the terminal window (compiled with MCUX):

begin post-processing
         Class ID = [1][person]
:        Class ID = [2][bicycle]
[...]
:        Class ID = [16][bird]
:        Class ID = [17][cat]
:        Class ID = [18][dog]
:        Class ID = [19][horse]
                Predicted bounding box[0]: 0.316 0.061 0.900 0.408 (0.966182)
                Predicted bounding box[1]: 0.070 0.323 0.890 0.657 (0.909269)
                Predicted bounding box[2]: 0.475 0.628 0.798 0.845 (0.812548)
                Predicted bounding box[3]: 0.468 0.837 0.821 0.998 (0.778532)
         Class ID = [20][sheep]
:        Class ID = [21][cow]
:        Class ID = [24][zebra]
[...]
:        Class ID = [90][toothbrush]
: decode img takes 54000 us, inference takes 2378000 us


