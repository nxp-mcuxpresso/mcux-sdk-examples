# io-att-driver v4

> An Audio Tuning Tool driver to brings audio streamer extensible control and EAP parameters get/set feature.

### Board requirements
- Platform independent

## Documentation
The ATT driver "eap_att.h/c" which is used to provide internal EAP parameters structures and extensible mechanism
to connect into custom audio stream control feature in various kind of software.

This driver contains state machine which could be linked with play/pause/stop/... features inside your code to obtain full control
from Audio Tuning Tool. If these features not needed on your side then all handlers (i.e. eap_att_control_t.play|pause|resume|...)
should be defined as function which returns kEapAttCodeOk directly.

In the case that custom application has its own LVM params definitions (not using eap_att_control_t.<inst|headroom|control>Params)
then at least LVM_Handle_t has to be registered (use eap_att_register_handle(...) function) and eap_att_process() should be polled
periodically (note: this causes that Audio Tuning Tool can call LVM set/get methods when needed - during push/pull from the UI; for more
info please see ATT_driver/eap_att.c at the very end of this file is part marked as internal).

Your application can also maintain parameters synchronization itself. In this situation (also without audio stream control features)
the eap_att_process() is not necessary and Audio Tuning Tool will only read/write data in original reference at
eap_att_control_t.<inst|headroom|control>Params.

WARNING: If your code is touching these internal configuration structures, please be sure that your data are stored inside original
destination of these pointers (internal structures in eap_att.c) because communication with this driver now uses memory read/write
over debugger controlled by FreeMASTER so this communication channel operates directly with these internal structures on address defined
by a compiler inside ELF file. In other words, if you replace eap_att_control_t.<inst|headroom|control>Params with
another pointer then these new structure references could be ignored and push/pull from UI will still operate with original ones
(so use memcpy(..) or another data transfer mechanism instead)!!

## Conclusion
This ATT driver could be embedded into any kind of application bud without any integration it simply do nothing. So do not forget to 
create at least one separated process which will init driver and then call process function periodically.
 - eap_att_register_handle() - to register LVM_Handle_t
 - eap_att_process() - poll state machine
 - get_eap_att_control() - getter for ATT control structure

```c
void main(void) {
    printf("Main task entered.\n");
    
    LVM_Handle_t* handle; // obtain existing handle before registration 
    eap_att_register_handle(handle);
    
    while(true) {
        eap_att_process();
        vTaskDelay(1/ portTICK_PERIOD_MS); // let system to process something else 
    }
}
```
This meta-code could be used as illustration for embedders. If you are looking for more information please look into demo application 
documentation or code.

> ************************************************************
> Look into eap_att.h for more detailed info about driver API.
> ************************************************************

## History

### v4
Updated support for EAP library version readout.
### v2
Added ability to read the EAP library version. Integrated API to control device master volume.
### v1
Initial release with state machine and fully extensible command handlers.

## License

This software is owned or controlled by NXP Semiconductors.
Use of this software is governed by the BSD-3-Clause License distributed with this material.

See the `LICENSE.txt` file distributed for more details.

---

Author Michal Kelnar,
Copyright 2021 [NXP](http://nxp.com/)
