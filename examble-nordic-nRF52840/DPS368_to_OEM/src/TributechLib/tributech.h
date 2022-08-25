/*
 * tributech.h
 *
 *  Created on: 13 Jul 2022
 *      Author: DanielHackl
 */

#ifndef LIBRARIES_OEMLIB_TRIBUTECHLIB_TRIBUTECH_H_
#define LIBRARIES_OEMLIB_TRIBUTECHLIB_TRIBUTECH_H_

extern bool configuration_received;
extern uint32_t get_config_transactionnr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parse oem response and save configuration - return true if success
uint8_t parse_oem_response_save_configuration(char * data, uint16_t cmd_len);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parse ValueMetaDataID from configuration - return true if success
uint8_t get_valueMetaDataId(char * stream_name, char * id);

#endif /* LIBRARIES_OEMLIB_TRIBUTECHLIB_TRIBUTECH_H_ */
