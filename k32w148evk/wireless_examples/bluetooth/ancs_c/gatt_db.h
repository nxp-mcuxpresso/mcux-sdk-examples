PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
        CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c) )
            VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x00, 0x00, 0x00, 0x00)
            CCCD(cccd_service_changed)

PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
    CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c | gGattCharPropWrite_c) )
            VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 14, "NXP_BLE_ANCS_C")
    CHARACTERISTIC(char_appearance, gBleSig_GapAppearance_d, (gGattCharPropRead_c) )
            VALUE(value_appearance, gBleSig_GapAppearance_d, (gPermissionFlagReadable_c), 2, UuidArray(gGenericWatch_c))
    CHARACTERISTIC(char_ppcp, gBleSig_GapPpcp_d, (gGattCharPropRead_c) )
            VALUE(value_ppcp, gBleSig_GapPpcp_d, (gPermissionFlagReadable_c), 8, 0x0A, 0x00, 0x10, 0x00, 0x64, 0x00, 0xE2, 0x04)
    CHARACTERISTIC(char_security_levels, gBleSig_GattSecurityLevels_d, (gGattCharPropRead_c) )
            VALUE(value_security_levels, gBleSig_GattSecurityLevels_d, (gPermissionFlagReadable_c), 2, 0x01, 0x01)

PRIMARY_SERVICE(service_battery, gBleSig_BatteryService_d)
    CHARACTERISTIC(char_battery_level, gBleSig_BatteryLevel_d, (gGattCharPropNotify_c | gGattCharPropRead_c))
        VALUE(value_battery_level, gBleSig_BatteryLevel_d, (gPermissionFlagReadable_c), 1, 0x5A)
        DESCRIPTOR(desc_bat_level, gBleSig_CharPresFormatDescriptor_d, (gPermissionFlagReadable_c), 7, 0x04, 0x00, 0xAD, 0x27, 0x01, 0x01, 0x00)
        CCCD(cccd_battery_level)

PRIMARY_SERVICE(service_device_info, gBleSig_DeviceInformationService_d)
    CHARACTERISTIC(char_manuf_name, gBleSig_ManufacturerNameString_d, (gGattCharPropRead_c) )
        VALUE(value_manuf_name, gBleSig_ManufacturerNameString_d, (gPermissionFlagReadable_c), sizeof(MANUFACTURER_NAME)-1, MANUFACTURER_NAME)
    CHARACTERISTIC(char_model_no, gBleSig_ModelNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_model_no, gBleSig_ModelNumberString_d, (gPermissionFlagReadable_c), 16, "ANCS Client Demo")
    CHARACTERISTIC(char_serial_no, gBleSig_SerialNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_serial_no, gBleSig_SerialNumberString_d, (gPermissionFlagReadable_c), 7, "BLESN01")
    CHARACTERISTIC(char_hw_rev, gBleSig_HardwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_hw_rev, gBleSig_HardwareRevisionString_d, (gPermissionFlagReadable_c), sizeof(BOARD_NAME)-1, BOARD_NAME)
    CHARACTERISTIC(char_fw_rev, gBleSig_FirmwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_fw_rev, gBleSig_FirmwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.1")
    CHARACTERISTIC(char_sw_rev, gBleSig_SoftwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_sw_rev, gBleSig_SoftwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.4")
    CHARACTERISTIC(char_system_id, gBleSig_SystemId_d, (gGattCharPropRead_c) )
        VALUE(value_system_id, gBleSig_SystemId_d, (gPermissionFlagReadable_c), 8, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x9F, 0x04, 0x00)

PRIMARY_SERVICE(service_current_time, gBleSig_CurrentTimeService_d)
    CHARACTERISTIC(char_current_time, gBleSig_CurrentTime_d, (gGattCharPropNotify_c | gGattCharPropWrite_c | gGattCharPropRead_c))
        VALUE_VARLEN(value_current_time, gBleSig_CurrentTime_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 10, 2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
        CCCD(cccd_current_time)
    CHARACTERISTIC(char_local_time_info, gBleSig_LocalTimeInformation_d, (gGattCharPropRead_c) )
        VALUE(value_local_time_info, gBleSig_LocalTimeInformation_d, (gPermissionFlagReadable_c), 2, 0x00, 0x00)
    CHARACTERISTIC(char_reference_time_info, gBleSig_ReferenceTimeInformation_d, (gGattCharPropRead_c) )
        VALUE(value_reference_time_info, gBleSig_ReferenceTimeInformation_d, (gPermissionFlagReadable_c), 4, 0x00, 0x00, 0x00, 0x00)

PRIMARY_SERVICE(service_reference_time, gBleSig_ReferenceTimeUpdateService_d)
    CHARACTERISTIC(char_time_update_cp, gBleSig_TimeUpdateControlPoint_d, (gGattCharPropWriteWithoutRsp_c))
        VALUE(value_time_update_cp, gBleSig_TimeUpdateControlPoint_d, (gPermissionFlagWritable_c), 1, 0x00)
    CHARACTERISTIC(char_time_update_state, gBleSig_TimeUpdateState_d, (gGattCharPropRead_c))
        VALUE(value_time_update_state, gBleSig_TimeUpdateState_d, (gPermissionFlagReadable_c), 2, 0x00, 0x00)

PRIMARY_SERVICE(service_next_dst, gBleSig_NextDSTChangeService_d)
    CHARACTERISTIC(char_time_with_dst, gBleSig_TimeWithDST_d, (gGattCharPropRead_c))
        VALUE(value_time_with_dst, gBleSig_TimeWithDST_d, (gPermissionFlagReadable_c), 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF)

/* Placed at end of file due to macro interpretation of line numbers. */
/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */