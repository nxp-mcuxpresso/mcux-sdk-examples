PRIMARY_SERVICE(service_gatt, gBleSig_GenericAttributeProfile_d)
        CHARACTERISTIC(char_service_changed, gBleSig_GattServiceChanged_d, (gGattCharPropIndicate_c) )
            VALUE(value_service_changed, gBleSig_GattServiceChanged_d, (gPermissionNone_c), 4, 0x00, 0x00, 0x00, 0x00)
            CCCD(cccd_service_changed)

PRIMARY_SERVICE(service_gap, gBleSig_GenericAccessProfile_d)
    CHARACTERISTIC(char_device_name, gBleSig_GapDeviceName_d, (gGattCharPropRead_c) )
        VALUE(value_device_name, gBleSig_GapDeviceName_d, (gPermissionFlagReadable_c), 11, "NXP_BLE_CAR")
    CHARACTERISTIC(char_security_levels, gBleSig_GattSecurityLevels_d, (gGattCharPropRead_c) )
        VALUE(value_security_levels, gBleSig_GattSecurityLevels_d, (gPermissionFlagReadable_c), 2, 0x01, 0x04)

PRIMARY_SERVICE(service_dk, gBleSig_CCC_DK_UUID_d)
    CHARACTERISTIC_UUID128(char_vehicle_psm, uuid_char_vehicle_psm, (gGattCharPropRead_c) )
            VALUE_UUID128(value_vehicle_psm, uuid_char_vehicle_psm, (gPermissionFlagReadable_c), 2, MSB2(gDK_DefaultVehiclePsm_c), LSB2(gDK_DefaultVehiclePsm_c))
    CHARACTERISTIC(char_tx_power_level, gBleSig_TxPower_d, (gGattCharPropRead_c) )
            VALUE(value_tx_power_level, gBleSig_TxPower_d, (gPermissionFlagReadable_c), 1, 0x01)
    CHARACTERISTIC_UUID128(char_antenna_id, uuid_char_antenna_id, (gGattCharPropRead_c) )
            VALUE_UUID128(value_antenna_id, uuid_char_antenna_id, (gPermissionFlagReadable_c), 2, 0x02, 0x03)

/* Placed at the end of file due to macro interpretation of line numbers. */
/*! *********************************************************************************
* \file gatt_db.h
*
* Copyright 2021-2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */