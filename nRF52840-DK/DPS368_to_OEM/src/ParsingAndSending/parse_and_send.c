/*
 * parse_and_send.c
 *
 *  Created on: 28 May 2022
 *      Author: A.Ascher
 */

#include <parse_and_send.h>
#include <zephyr.h>
#include <stdlib.h>
#include <stdio.h>
#include <Base64.h>
#include <dps368.h>
#include <scheduling.h>
#include <uart_communication.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(parse_and_send, LOG_LEVEL_INF);

char tmp_value_string[30];
char psr_value_string[30];
uint32_t transaction_nr;

void send_tmp_and_psr_value_to_oem(void)
{
    if(psr_available != true && tmp_available != true)
    {
        return;
    }

    // Temperature definitions
    char valuemetadataid_tmp[100] = "95817384-7cec-4bb5-895f-b07c305b9d50";
    char *               response_tmp_psr;
    uint16_t             base64_tmp_length;            // base64 length
    char                 *base64_tmp_string;           // pointer to base64 string
    float temp_tmp_val = data.tmp_val;

    base64_tmp_string = 0x0;			// reset pointer

    base64_tmp_string = base64_encode(&temp_tmp_val, sizeof(float), &base64_tmp_length);

    // Pressure definitions
    char valuemetadataid_psr[100] = "0df64d80-a8b4-4154-92bf-7e7a82794de0";
    uint16_t        base64_psr_length;            // base64 length
    char            *base64_psr_string;           // pointer to base64 string
    float           temp_psr_val = data.psr_val;

    base64_psr_string = 0x0;			// reset pointer

    response_tmp_psr = k_calloc(500,sizeof(char));
    base64_psr_string = base64_encode(&temp_psr_val, sizeof(float), &base64_psr_length);

    
    // Send values
    size_t response_tmp_psr_len = sprintf(response_tmp_psr, "{\"TransactionNr\": %"PRIu32",\"Operation\": \"ProvideValues\",\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}],\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}]}\r\n" , transaction_nr, valuemetadataid_tmp, base64_tmp_string, valuemetadataid_psr, base64_psr_string);
    LOG_INF("%s", response_tmp_psr);
    uart_tx(uart, response_tmp_psr, response_tmp_psr_len+1, 500);

    transaction_nr++;

    k_free(response_tmp_psr);
    k_free(base64_tmp_string);
    k_free(base64_psr_string);
    psr_available = false;
    tmp_available = false;
}