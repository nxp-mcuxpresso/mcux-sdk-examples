/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 * 
 */

/*!\file ncp_host_command_ble.h
 *\brief NCP Bluetooth LE host command interfaces.
 */

#ifndef __NCP_HOST_COMMAND_BLE_H_
#define __NCP_HOST_COMMAND_BLE_H_

#include "ncp_adapter.h"
#include "ncp_host_app.h"
#include "ncp_host_command.h"
#include "ncp_cmd_ble.h"

/** 
 * This API is used to set Bluetooth LE advertising data
 * 
 * \param[in] argc  Argument count, the number of strings pointed to by argv. \n
 *                  argc should be 2
 * \param[in] argv  Argument vector. \n
 *                  argv[0]: 'ble-set-adv-data' \n
 *                  argv[1]: Advertising data to set (Required) \n
 *                          Advertising data should follow struture: \n
 *                          \struct bt_data { \n 
 *                              U8 len; \n
 *                              U8 type; \n
 *                              const U8 *data; \n
 *                          } \n 
 *                          Example: 0e094e43505f444542554731323334 \n
 *                          Detail as: \n
 *                              len : 0e(14 bytes, contain type and data) \n
 *                              type: 09(BT_DATA_NAME_COMPLETE) \n
 *                              data: 4e43505f444542554731323334(NCP_DEBUG1234) \n
 *                           TYPE Reference: Core Specification Supplement, Part A, Data Types Specification 
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 * 
 */
int ble_set_adv_data_command(int argc, char **argv);

/** This API is used to start Bluetooth LE advertising
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 1 
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-start-adv'
 *                 
 * 
 * \return NCP_STATUS_SUCCESS if success
 */
int ble_start_adv_command(int argc, char **argv);

/** This API is used to stop Bluetooth LE advertising
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 1
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-stop-adv'
 * 
 * \return NCP_STATUS_SUCCESS
 */
int ble_stop_adv_command(int argc, char **argv);

/** This API is used to set Bluetooth LE scan parameter
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 4
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-set-scan-param' \n
 *                 argv[1]: filter option (Required) \n
 *                          0 -- BT_LE_SCAN_OPT_NONE (Convenience value when no options are specified) \n
 *                          1 -- BT_LE_SCAN_OPT_FILTER_DUPLICATE (Filter duplicates) \n
 *                          2 -- BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST (Filter using filter accept list) \n
 *                 argv[2]: scan interval (Required) \n
 *                          Range from 4 to 16384 (Decimal value) \n
 *                 argv[3]: scan window (Required) \n
 *                          Range: 4 to 16384 (Decimal value, window value shall be less than or equal to interval value) \n
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_set_scan_param_command(int argc, char **argv);

/** This API is used to start Bluetooth LE scanning
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 2
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-start-scan' \n
 *                 argv[1] -- scan type (Required) \n
 *                            0 -- active scan \n
 *                            1 -- passive scan
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_start_scan_command(int argc, char **argv);

/** This API is used to stop Bluetooth LE scanning
 * \param[in] argc Argument count, the number of strings pointed to by argv.
 *                 argc should be 1
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-stop-scan' \n
 * 
 * \return NCP_STATUS_SUCCESS
 */
int ble_stop_scan_command(int argc, char **argv);

/** This API is used to connect the advertising device
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 3
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-connect' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_connect_command(int argc, char **argv);

/** This API is used to disconnect the connected device
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 3
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-disconnect' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_disconnect_command(int argc, char **argv);

/** This API is used to update the device connection parameter
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 7
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-conn-param-update' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: connection maximum interval (Required) \n
 *                          Maximum value for the connection interval. This shall be greater than or equal to minimum connection interval. (Decimal value) \n
 *                 argv[4]: connection minimum interval (Required) \n
 *                          Minimum value for the connection interval. This shall be less than or equal to maximum connection interval. (Decimal value) \n
 *                 argv[5]: connection latency \n
 *                          Maximum Peripheral latency for the connection in number of subrated connection events. (Decimal value) \n
 *                 argv[6]: connection timeout \n
 *                          Supervision timeout for the Bluetooth LE link. (Decimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_connect_paramter_update_command(int argc, char **argv);

/** This API is used to set the Bluetooth PHY
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 5
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-set-phy' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: Bluetooth TX PHY \n
 *                          1 -- 1M PHY \n
 *                          2 -- 2M PHY \n
 *                          4 -- Coded PHY \n
 *                 argv[4]: Bluetooth RX PHY \n
 *                          1 -- 1M PHY \n
 *                          2 -- 2M PHY \n
 *                          4 -- Coded PHY
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_set_phy_command(int argc, char **argv);

/** This API is used to update data packet length
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 4 or 5
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-set-data-len' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: Maximum data length to transmit (Required) \n
 *                          Range: 27 to 251 (Decimal value) \n
 *                 argv[4]: Maximum TX transimit time (Optional) \n
 *                          Range: 328 to 17040 (Decimal value, this parameter can be left unset when sending commands) 
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_set_data_len_command(int argc, char **argv);

/** This API is used to configure the filter list. After the filter list is set, use the ble-scan-param to scan the parameter. Select the filter option 2. Send the scan command.
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be at least 4
 * \param[in] argv Argument vector. 
 *                 argv[0]: 'ble-set-filter-list' \n
 *                 argv[1]: The number of device address need to filter (Required) \n
 *                          Range: 1 to 255 (Decimal value) \n
 *                 argv[2]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[3]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 ...... \n
 *                 argv[n-1]: address type (Optional) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[n]: device address (Optional) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_set_filter_list_command(int argc, char **argv);

/** This API is used to initiate security connection. If peer is already paired, IUT (Implementation under test) is expected to enable security
 *  (encryption) with peer. If peer is not paired, the IUT start the pairing process.
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 3
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-start-encryption' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_start_encryption_command(int argc, char **argv);

/** This API is used to read profile charactertistic value by handle. This API must be called after the device is connected
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 4
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-read-characteristic' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: attribute handle (Required) \n
 *                          Range from 0x0000 ~ 0xffff (Hexadecimal value)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_read_characteristic_command(int argc, char **argv);

/** This API is used to register the service that runs the profile on the NCP device
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be at least 3
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-register-service' \n
 *                 argv[1]: The number of service to register (Required) \n
 *                          Range from 1 to 5 \n
 *                 argv[2]: service id (Required) \n
 *                          1 -- Peripheral HTS (Health Thermometer) \n
 *                          2 -- Peripheral HRS (Health Rate) \n
 *                          3 -- Peripheral BAS (Battery Service) \n
 *                          4 -- Central HTS \n
 *                          5 -- Central HRS \n
 *                 ..... \n
 *                 argv[n]: service id (Optional)
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_register_service_command(int argc, char **argv);

/** This API is used to set the device power mode
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 2
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-set-power-mode' \n
 *                 argv[1]: device power mode \n
 *                          0 -- enable controller auto sleep \n
 *                          1 -- disable controller auto sleep
 * 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_set_power_mode_command(int argc, char **argv);

/** This API is used to set the service parameters. One service may have several characteristic
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be at least 11
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-host-svc-add' \n
 *                 argv[1]: 'prim' (Required) \n
 *                 argv[2]: primary service uuid (Required) \n
 *                          XXXX (Hexadecimal value) \n
 *                 argv[3]: 'chrc' (Required) \n
 *                 argv[4]: characteristic uuid (Required) \n
 *                          XXXX (Hexadecimal value) \n
 *                 argv[5]: characteristic properties (Required) \n
 *                          XX (Hexadecimal value) \n
 *                          BIT 1: Broadcast  \n
                            BIT 2: Read \n
                            BIT 3: Write Without Response \n
                            BIT 4: Write \n
                            BIT 5: Notify \n
                            BIT 6: Indicate \n
                            BIT 7: Authenticated Signed Writes \n
                            BIT 8: Extended Properties \n
 *                 argv[6]: characteristic permission (Required) \n
 *                          XX (Hexadecimal value) \n
 *                          0:   None \n
                            BIT 0: Read \n
                            BIT 1: Write \n
                            BIT 2: Read with Encryption \n
                            BIT 3: Write with Encryption \n
                            BIT 4: Read with Authentication \n
                            BIT 5: Write with Authentication \n
                            BIT 6: Prepare Write \n
                            BIT 7: Read with LE Secure Connection encryption \n
                            BIT 8: Write with LE Secure Connection encryption \n
 *                 argv[7]: 'ccc' (Required) \n
 *                 argv[8]: client characteristic configuration uuid (Required) \n
 *                          XXXX (Hexadecimal value) \n
 *                 argv[9]: client characteristic configuration permissions (Required) \n
 *                          XX (Hexadecimal value) \n
                            0:   None \n
                            BIT 0: Read \n
                            BIT 1: Write \n
                            BIT 2: Read with Encryption \n
                            BIT 3: Write with Encryption \n
                            BIT 4: Read with Authentication \n
                            BIT 5: Write with Authentication \n
                            BIT 6: Prepare Write \n
                            BIT 7: Read with LE Secure Connection encryption \n
                            BIT 8: Write with LE Secure Connection encryption \n
 *                 ...... \n
 *                argv[n]: 'start' (Required)  \n   
 *                  \n
 *                Example: Add hts service  \n
 *                         ble-host-svc-add prim 1809 chrc 2a1c 20 00 ccc 2902 03 start (same as the command 'ble-start-service hts' )
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_host_service_add_command(int argc, char **argv);

/** This API is used to start service that runs the profile on the NCP host
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 2
 * \param[in] argv Argument vector.
 *                 argv[0]: 'ble-start-service' \n
 *                 argv[1]: profile name (Required) \n
 *                          'hts' -- Peripheral health thermometer \n
 *                          'htc' -- Client health thermometer \n
 *                          'hrs' -- Peripheral health rate \n
 *                          'hrc' -- Client health rate \n
 *                          'bas' -- Peripheral battery service \n
 *                 \note To use the command, enable the following macros: \n
 *                       CONFIG_NCP_BLE \n
                         CONFIG_NCP_BLE_PROFILE_MODE \n
                         CONFIG_NCP_HTS \n
                         CONFIG_NCP_HRS \n
                         CONFIG_NCP_HTC \n
                         CONFIG_NCP_HRC \n
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_start_service_command(int argc, char **argv);

/** This API is used to subscribe the ccc handle
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 6
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-cfg-subscribe' \n
 *                 argv[1]: property type (Required) \n
 *                          'indicate' -- If set, changes to Characteristic Value are indicated \n
 *                          'notify' -- If set, changes to Characteristic Value are notified \n
 *                 argv[2]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[3]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[4]: enable subscription option (Required) \n
 *                          0 -- disable subscription \n
 *                          1 -- enable subscription \n
 *                 argv[5]: client characteristic configuration handle (Required) \n
 *                          specific one needs to be referred to the print ccc handle value of Discover the Descriptor Event (Hexadecimal value)
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_cfg_subscribe_command(int argc, char **argv);

/** This API is used to connect the l2cap PSM (Protocol service multiplexer). \n
 *  The l2cap PSM need to be registered by remote device, then issue ble-l2cap-connect command to connect the remote device
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 4
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-l2cap-connect' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: Protocol service multiplexer value (Required) \n
 *                          Range: 128 to 255 (Decimal value)
 *                 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_l2cap_connection_command(int argc, char **argv);

/** This API is used to disconnect the l2cap channel
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 3
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-l2cap-disconnect' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value)
 *                 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_l2cap_disconnect_command(int argc, char **argv);

/** This API is used to send the data with the l2cap PSM
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 4
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-l2cap-send' \n
 *                 argv[1]: address type (Required) \n
 *                          'public' -- public address \n
 *                          'random' -- random address \n
 *                 argv[2]: device address (Required) \n
 *                          device address format: XX:XX:XX:XX:XX:XX (Hexadecimal value) \n
 *                 argv[3]: times \n
 *                          the number of times a loop is sent (Decimal value)
 *                 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_l2cap_send_command(int argc, char **argv);

/** This API is used to register the l2cap PSM 
 * \param[in] argc Argument count, the number of strings pointed to by argv. \n
 *                 argc should be 2
 * \param[in] argv Argument vector. \n
 *                 argv[0]: 'ble-l2cap-register' \n
 *                 argv[1]: Protocol service multiplexer value (Required) \n
 *                          Range: 128 to 255 (Decimal value)
 *                 
 * \return NCP_STATUS_SUCCESS if success
 * \return NCP_STATUS_ERROR if failure
 */
int ble_l2cap_register_command(int argc, char **argv);

/**
 * 
 * Get NCP Bluetooth LE command buffer
 * 
 * \return A ponter to MCU_NCPCmd_DS_COMMAND
 * 
 */
MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_ble(void);

#endif /*__NCP_HOST_COMMAND_BLE_H_*/