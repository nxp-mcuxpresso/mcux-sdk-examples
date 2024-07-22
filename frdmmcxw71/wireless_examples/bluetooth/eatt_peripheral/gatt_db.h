PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
    CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c))
        VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c), 16, "EATT Peripheral")
    CHARACTERISTIC(char_security_levels, gBleSig_GattSecurityLevels_d, (gGattCharPropRead_c) )
        VALUE(value_security_levels, gBleSig_GattSecurityLevels_d, (gPermissionFlagReadable_c), 2, 0x01, 0x03)

PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
    CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c))
        VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x01, 0x00, 0xFF, 0xFF)
        CCCD(cccd_service_changed)
    CHARACTERISTIC(char_client_supported_features, gBleSig_GattClientSupportedFeatures_d, (gGattCharPropRead_c | gGattCharPropWrite_c))
        VALUE(value_client_supported_features, gBleSig_GattClientSupportedFeatures_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
    CHARACTERISTIC(char_database_hash, gBleSig_GattDatabaseHash_d, (gGattCharPropRead_c))
        VALUE(value_database_hash, gBleSig_GattDatabaseHash_d, (gPermissionFlagReadable_c), 16, 0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00)
    CHARACTERISTIC(char_server_supported_features, gBleSig_GattServerSupportedFeatures_d, (gGattCharPropRead_c))
        VALUE(value_server_supported_features, gBleSig_GattServerSupportedFeatures_d, (gPermissionFlagReadable_c), 1, 0x03)

PRIMARY_SERVICE(service_battery, gBleSig_BatteryService_d)
    CHARACTERISTIC(char_battery_level, gBleSig_BatteryLevel_d, (gGattCharPropNotify_c | gGattCharPropRead_c))
        VALUE(value_battery_level, gBleSig_BatteryLevel_d, (gPermissionFlagReadable_c), 1, 0x5A)
        DESCRIPTOR(desc_bat_level, gBleSig_CharPresFormatDescriptor_d, (gPermissionFlagReadable_c), 7, 0x04, 0x00, 0xAD, 0x27, 0x01, 0x01, 0x00)
        CCCD(cccd_battery_level)

PRIMARY_SERVICE(service_device_info, gBleSig_DeviceInformationService_d)
    CHARACTERISTIC(char_manuf_name, gBleSig_ManufacturerNameString_d, (gGattCharPropRead_c) )
        VALUE(value_manuf_name, gBleSig_ManufacturerNameString_d, (gPermissionFlagReadable_c), sizeof(MANUFACTURER_NAME)-1, MANUFACTURER_NAME)
    CHARACTERISTIC(char_model_no, gBleSig_ModelNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_model_no, gBleSig_ModelNumberString_d, (gPermissionFlagReadable_c), 16, "EATT Peripheral")
    CHARACTERISTIC(char_serial_no, gBleSig_SerialNumberString_d, (gGattCharPropRead_c) )
        VALUE(value_serial_no, gBleSig_SerialNumberString_d, (gPermissionFlagReadable_c), 7, "BLESN01")
    CHARACTERISTIC(char_hw_rev, gBleSig_HardwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_hw_rev, gBleSig_HardwareRevisionString_d, (gPermissionFlagReadable_c), sizeof(BOARD_NAME)-1, BOARD_NAME)
    CHARACTERISTIC(char_fw_rev, gBleSig_FirmwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_fw_rev, gBleSig_FirmwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.1")
    CHARACTERISTIC(char_sw_rev, gBleSig_SoftwareRevisionString_d, (gGattCharPropRead_c) )
        VALUE(value_sw_rev, gBleSig_SoftwareRevisionString_d, (gPermissionFlagReadable_c), 5, "1.1.4")

PRIMARY_SERVICE(service_A, 0xA00A)
    CHARACTERISTIC(char_service_A1, gBleSig_Report_d, (gGattCharPropRead_c | gGattCharPropIndicate_c | gGattCharPropWrite_c) )
        VALUE(value_A1, gBleSig_Report_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 2, 0x00, 0x00)
        CCCD(cccd_A1)
    CHARACTERISTIC(char_service_A2, 0xA002, (gGattCharPropRead_c | gGattCharPropWrite_c))
        VALUE(value_A2, 0xA002, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 50, "11111222223333344444555556666677777888889999900000")
    CHARACTERISTIC(char_service_A3, 0xA003, (gGattCharPropWrite_c))
        VALUE(value_A3, 0xA003, (gPermissionFlagWritable_c), 1, 0x03)
    CHARACTERISTIC(char_A1_control_point, 0xA001, (gGattCharPropWrite_c))
        VALUE(value_A1_control_point, 0xA001, (gPermissionFlagWritable_c), 0x01, 0x00)


PRIMARY_SERVICE(service_B, 0xB00B)
    CHARACTERISTIC(char_service_B1, gBleSig_Report_d, (gGattCharPropRead_c | gGattCharPropIndicate_c | gGattCharPropWrite_c))
        VALUE(value_B1, gBleSig_Report_d, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 2, 0x00, 0x00)
        CCCD(cccd_B1)
    CHARACTERISTIC(char_service_B2, 0xB002, (gGattCharPropRead_c | gGattCharPropWrite_c))
        VALUE(value_B2, 0xB002, (gPermissionFlagReadable_c | gPermissionFlagWritable_c), 10, "0123456789")
    CHARACTERISTIC(char_service_B3, 0xB003, (gGattCharPropWrite_c))
        VALUE(value_B3, 0xB003, (gPermissionFlagWritable_c), 2, 0x00, 0x01)
    CHARACTERISTIC(char_B1_control_point, 0xB001, (gGattCharPropWrite_c) )
        VALUE(value_B1_control_point, 0xB001, (gPermissionFlagWritable_c), 1, 0x00)

/* Placed at end of file due to macro interpretation of line numbers. */
/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */