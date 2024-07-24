
PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
        CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c) )
            VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x00, 0x00, 0x00, 0x00)
            CCCD(cccd_service_changed)

PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
        CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c) )
            VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c), 12, "NXP_BLE_HIDH")
        CHARACTERISTIC(char_appearance, gBleSig_GapAppearance_d, (gGattCharPropRead_c) )
            VALUE(value_appearance, gBleSig_GapAppearance_d, (gPermissionFlagReadable_c), 2, UuidArray(gGenericComputer_c))
        CHARACTERISTIC(char_security_levels, gBleSig_GattSecurityLevels_d, (gGattCharPropRead_c) )
            VALUE(value_security_levels, gBleSig_GattSecurityLevels_d, (gPermissionFlagReadable_c), 2, 0x01, 0x01)

/* Placed at end of file due to macro interpretation of line numbers. */
/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */