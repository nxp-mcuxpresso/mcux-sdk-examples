PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
        CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c) )
            VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x00, 0x00, 0x00, 0x00)
            CCCD(cccd_service_changed)

PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
    CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c) )
            VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c), 9, "NXP_SHELL")
    CHARACTERISTIC(char_appearance, gBleSig_GapAppearance_d, (gGattCharPropRead_c) )
            VALUE(value_appearance, gBleSig_GapAppearance_d, (gPermissionFlagReadable_c), 2, 0x00, 0x00)
    CHARACTERISTIC(char_security_levels, gBleSig_GattSecurityLevels_d, (gGattCharPropRead_c) )
            VALUE(value_security_levels, gBleSig_GattSecurityLevels_d, (gPermissionFlagReadable_c), 2, 0x01, 0x01)

PRIMARY_SERVICE_UUID128(service_throughput, uuid_service_throughput)
    CHARACTERISTIC_UUID128(char_throughput_stream, uuid_throughput_stream, (gGattCharPropWriteWithoutRsp_c))
        VALUE_UUID128_VARLEN(value_throughput_stream, uuid_throughput_stream, (gPermissionFlagWritable_c), gAttMaxWriteDataSize_d(gAttMaxMtu_c), 1, 0x00)

PRIMARY_SERVICE(service_battery, gBleSig_BatteryService_d)
    CHARACTERISTIC(char_battery_level, gBleSig_BatteryLevel_d, (gGattCharPropNotify_c | gGattCharPropRead_c))
        VALUE(value_battery_level, gBleSig_BatteryLevel_d, (gPermissionFlagReadable_c), 1, 0x5A)
        DESCRIPTOR(desc_bat_level, gBleSig_CharPresFormatDescriptor_d, (gPermissionFlagReadable_c), 7, 0x04, 0x00, 0xAD, 0x27, 0x01, 0x00, 0x00)
        CCCD(cccd_battery_level)

PRIMARY_SERVICE(service_device_info, gBleSig_DeviceInformationService_d)
    CHARACTERISTIC(char_manuf_name, gBleSig_ManufacturerNameString_d, (gGattCharPropRead_c) )
        VALUE(value_manuf_name, gBleSig_ManufacturerNameString_d, (gPermissionFlagReadable_c), sizeof(MANUFACTURER_NAME)-1, MANUFACTURER_NAME)
    CHARACTERISTIC(char_model_no, gBleSig_ModelNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_model_no, gBleSig_ModelNumberString_d, (gPermissionFlagReadable_c), 14, "BLE Shell Demo")
    CHARACTERISTIC(char_serial_no, gBleSig_SerialNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_serial_no, gBleSig_SerialNumberString_d, (gPermissionFlagReadable_c), 7, "BLESN01")
    CHARACTERISTIC(char_hw_rev, gBleSig_HardwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_hw_rev, gBleSig_HardwareRevisionString_d, (gPermissionFlagReadable_c), sizeof(BOARD_NAME)-1, BOARD_NAME)
    CHARACTERISTIC(char_fw_rev, gBleSig_FirmwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_fw_rev, gBleSig_FirmwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.1")
    CHARACTERISTIC(char_sw_rev, gBleSig_SoftwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_sw_rev, gBleSig_SoftwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.3")

/* Placed at end of file due to macro interpretation of line numbers. */
/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */