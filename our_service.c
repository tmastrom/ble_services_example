/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdint.h>
#include <string.h>
#include "our_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_our_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
  	ble_os_t * p_our_service =(ble_os_t *) p_context;  
		// OUR_JOB: Step 3.D Implement switch case handling BLE events related to our service. 
	switch (p_ble_evt->header.evt_id)
        {
            case BLE_GAP_EVT_CONNECTED:
                p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
                break;
            case BLE_GAP_EVT_DISCONNECTED:
                p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
                break;
            default:
                break;

        }
}


/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
static uint32_t our_char_add(ble_os_t * p_our_service)
{
    // OUR_JOB: Step 2.A, Add a custom characteristic UUID
    uint32_t      err_code;
    ble_uuid_t    char_uuid;
    ble_uuid128_t base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid = BLE_UUID_OUR_CHARACTERISTC_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);
 

    // OUR_JOB: Step 2.F Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;


    //  Step 3.A, Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    /*  Declare metadata structure for CCCD
    *   Set security to open
    *   Store descriptor in the SD part of memory
    *   Store data in cccd_md struct and enable notification
    */
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc              = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md         = &cccd_md;
    char_md.char_props.notify = 1;


    // OUR_JOB: Step 2.B, Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc  = BLE_GATTS_VLOC_STACK;


    // OUR_JOB: Step 2.G, Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    

    // OUR_JOB: Step 2.C, Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;   // md means metadata


    // OUR_JOB: Step 2.H, Set characteristic length in number of bytes
    attr_char_value.max_len     = 4;
    attr_char_value.init_len    = 4;
    uint8_t value[4]            = {0x12, 0x34, 0x56, 0x78};
    attr_char_value.p_value     = value;


    // OUR_JOB: Step 2.E, Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &p_our_service->char_handles);
    APP_ERROR_CHECK(err_code);


    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void our_service_init(ble_os_t * p_our_service)
{

    // Declare 16 bit service and 128 bit base UUIDs and add them to BLE stack table 
    uint32_t    err_code;
    ble_uuid_t  service_uuid;
    ble_uuid128_t   base_uuid = BLE_UUID_OUR_BASE_UUID;
    service_uuid.uuid = BLE_UUID_OUR_SERVICE;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);    // add vendor specific UUID to the BLE stack
    APP_ERROR_CHECK(err_code);
   
 // OUR_JOB: Step 3.B, Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // Add our service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &p_our_service->service_handle);
	
    // Print messages to Segger Real Time Terminal
    SEGGER_RTT_WriteString(0, "Executing our_service_init().\n"); // Print message to RTT to the application flow
    SEGGER_RTT_printf(0, "Service UUID: 0x%#04x\n", service_uuid.uuid); // Print service UUID should match definition BLE_UUID_OUR_SERVICE
    SEGGER_RTT_printf(0, "Service UUID type: 0x%#02x\n", service_uuid.type); // Print UUID type. Should match BLE_UUID_TYPE_VENDOR_BEGIN. Search for BLE_UUID_TYPES in ble_types.h for more info
    SEGGER_RTT_printf(0, "Service handle: 0x%#04x\n", p_our_service->service_handle); // Print out the service handle. Should match service handle shown in MCP under Attribute values

    // OUR_JOB: Call the function our_char_add() to add our new characteristic to the service. 
    our_char_add(p_our_service);
}

// Function to be called when updating characteristic value
void our_temperature_characteristic_update(ble_os_t *p_our_service, int32_t *temperature_value)
{
    // Update characteristic value
    // check whether we are in a valid connection before trying to send out notifications
    
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t          len = 4;
        ble_gatts_hvx_params_t hvx_params;          // hvx: Handle Value X where X is either indication or notification
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->char_handles.value_handle; // what characteristic to work with
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;                // Notification vs indication
        hvx_params.offset = 0;                                        // select bytes not to transmit  
        hvx_params.p_len  = &len;                                     // number of bytes to transmit
        hvx_params.p_data = (uint8_t*)temperature_value;              // pointer to the actual data

        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);


    }


}