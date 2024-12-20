# config to select component, the format is CONFIG_USE_${component}
# Please refer to cmake files below to get available components:
#  ${SdkRootDirPath}/devices/MCXC444/all_lib_device.cmake

set(CONFIG_COMPILER gcc)
set(CONFIG_TOOLCHAIN armgcc)
set(CONFIG_USE_COMPONENT_CONFIGURATION false)
set(CONFIG_USE_middleware_freertos-kernel true)
set(CONFIG_USE_driver_clock true)
set(CONFIG_USE_driver_common true)
set(CONFIG_USE_device_MCXC444_CMSIS true)
set(CONFIG_USE_utility_debug_console true)
set(CONFIG_USE_component_lpuart_adapter true)
set(CONFIG_USE_component_serial_manager_uart true)
set(CONFIG_USE_component_serial_manager true)
set(CONFIG_USE_driver_lpuart true)
set(CONFIG_USE_component_lists true)
set(CONFIG_USE_device_MCXC444_startup true)
set(CONFIG_USE_driver_uart true)
set(CONFIG_USE_driver_port true)
set(CONFIG_USE_driver_smc true)
set(CONFIG_USE_driver_gpio true)
set(CONFIG_USE_utility_assert true)
set(CONFIG_USE_utilities_misc_utilities true)
set(CONFIG_USE_middleware_freertos-kernel_template true)
set(CONFIG_USE_middleware_freertos-kernel_extension true)
set(CONFIG_USE_CMSIS_Include_core_cm true)
set(CONFIG_USE_utility_str true)
set(CONFIG_USE_device_MCXC444_system true)
set(CONFIG_CORE cm0p)
set(CONFIG_DEVICE MCXC444)
set(CONFIG_BOARD frdmmcxc444)
set(CONFIG_KIT frdmmcxc444)
set(CONFIG_DEVICE_ID MCXC444)
set(CONFIG_FPU NO_FPU)
set(CONFIG_DSP NO_DSP)
