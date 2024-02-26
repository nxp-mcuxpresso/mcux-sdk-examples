# MCU Media Processing Pipeline

This is the documentation for the MCU Media Processing Pipeline API.

# Features Overview

The Media Processing Pipeline for MCUs is a software library for constructing media-handling components graphs for Vision-specific applications.

This is a clean and simple API which makes it easy to build and prototype vision-based applications

## Concept

The concept behind the API is to create a Media Processing Pipeline (MPP) based on processing elements.
The basic pipeline structure - the _mpp_ in the API context - has a chain/queue structure which begins with a **source element**:
- Camera
- Static image

The pipeline continues with multiple **processing elements** having a single input and a single output:
- Image format conversion
- Labeled rectangle drawing
- Machine learning inference with three frameworks:
   - Tensorflow Lite Micro 
   - GLOW
   - DeepViewRT

The pipeline can be closed by adding a **sink element**:
- Display panel
- Null sink
 
Also, multiple basic _mpps_ can be **joined** into a new one to which further elements can be added.
An _mpp_ can also be **split** when the same media stream must follow different processing paths.
With these join/split operations, more complex pipelines can be constructed.

Compatibilty of elements and supplied parameters are checked at each step and only compatible elements
can be added in an unequivocal way.

After the construction is complete, each _mpp_ must be started for all hardware and software
required to run the pipeline to initialize. Pipeline processing begins as soon as the the last start
call is flagged.

Each pipeline branch can be stopped individually. This involves stopping the execution and the hardware 
peripherals of the branch. After being stopped, each branch can be started again. 
To stop the whole pipeline you need to stop each of its branches separately.

At runtime the application receives events from the pipeline processing and may use these events
to update elements parameters. For example, in object detection when the label of a bounding
box must be updated whenever a new object is detected.

Summarizing, the application controls:
- Creation of the pipeline
- Instantiation of processing elements
- Connection of elements to each other
- Reception of callbacks based on specific events
- Updation of specific elements (not all elements can be updated)
- Stop of the pipeline (includes stop of the hardware peripherals).

Application does not control:
- Memory management
- Data structures management

The order in which an element is added to the pipeline defines its position within this pipeline, and therefore the order is important.

# How To
See provided examples/reference documentation for practical examples using the MPP API.

