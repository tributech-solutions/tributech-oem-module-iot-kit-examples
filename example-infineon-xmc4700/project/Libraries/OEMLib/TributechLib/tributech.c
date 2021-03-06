/*
 * tributech.c
 *
 *  Created on: 13 Jul 2022
 *      Author: DanielHackl
 */
#define CONFIGURATION_SIZE 4096
#include <DAVE.h>
#include "jsmn.h"
#include "inttypes.h"
#include "functions.h"

char configuration[CONFIGURATION_SIZE];
bool configuration_received;
uint32_t get_config_transactionnr;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parse oem response and save configuration - return true if success
uint8_t parse_oem_response_save_configuration(char * data, uint16_t cmd_len)
{
	uint16_t number_of_tokens = 0;	// number of tokens

	jsmn_parser p;					// parser
	jsmntok_t *t; 					// tokens
	char * key_name;				// object key name
	uint8_t length;					// length of key name
	uint16_t r_cmd_len = 0;			// length of received command phrase like "GetId" r_cmd_len = 5
	uint32_t received_transactionnr;// received transaction number

	if (cmd_len > CONFIGURATION_SIZE)
	{
		return false;
	}

	t = calloc(1024,sizeof(jsmntok_t));

	//+++++++++++++++++++++++++++++++++++++++++++
	// Initial start token
	p.pos = 0;
	p.toknext = 0;
	p.toksuper = -1;

	//+++++++++++++++++++++++++++++++++++++++++++
	// Get tokens by parsing json received text
	number_of_tokens = jsmn_parse(&p, data, cmd_len, t, 1024);

	//+++++++++++++++++++++++++++++++++++++++++++
	// invalid operation by less than 4 tokens
	if(number_of_tokens < 4 || number_of_tokens > 1024)
	{
		free(t);
		return false;
	}

	//+++++++++++++++++++++++++++++++++++++++++++
	// get transaction number
	for(uint8_t i = 1; i<number_of_tokens; i++)
	{
		length = t[i].end-t[i].start;
		key_name = calloc(length+1, sizeof(char));
		memcpy(key_name, &data[t[i].start], length);

		//+++++++++++++++++++++++++++++++++++++++
		// search for transaction number and sources
		if(strcmp(to_lower_case(key_name), "transactionnr") == 0)
		{
			received_transactionnr = (uint32_t)strtol(&data[(t[i+1].start)], NULL, 0);
		}
		if(strcmp(to_lower_case(key_name), "sources") == 0 && received_transactionnr == get_config_transactionnr)
		{
			//+++++++++++++++++++++++++++++++++++++++
			// save configuration as string
			memset(configuration,0x0,CONFIGURATION_SIZE);
			memcpy(configuration,data,cmd_len);
			configuration_received = true;

			free(key_name);
			free(t);
			return true;
		}
		free(key_name);
	}

	free(t);

	return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parse ValueMetaDataID from configuration - return true if success
uint8_t get_valueMetaDataId(char * stream_name, char * id)
{
	uint16_t number_of_tokens = 0;	// number of tokens

	jsmn_parser p;					// parser
	jsmntok_t *t; 					// tokens
	char operation[50];				// operation value
	char * key_name;				// object key name
	uint8_t length;					// length of key name
	uint16_t r_cmd_len = 0;			// length of received command phrase like "GetId" r_cmd_len = 5
	uint16_t object_stream_end_pos;				// end position of object stream
	uint16_t array_streams_end_pos;				// end position of array streams
	char id_temp[37];				// id temp
	char stream_name_temp[50];		// stream name temp

	t = calloc(1024,sizeof(jsmntok_t));

	//+++++++++++++++++++++++++++++++++++++++++++
	// Initial start token
	p.pos = 0;
	p.toknext = 0;
	p.toksuper = -1;

	//+++++++++++++++++++++++++++++++++++++++++++
	// Get tokens by parsing json received text
	number_of_tokens = jsmn_parse(&p, configuration, strlen(configuration), t, 1024);

	//+++++++++++++++++++++++++++++++++++++++++++
	// invalid operation by less than 4 tokens
	if(number_of_tokens < 4 || number_of_tokens > 1024)
	{
		free(t);
		return false;
	}

	//+++++++++++++++++++++++++++++++++++++++++++
	// get transaction number
	for(uint8_t j = 1; j<number_of_tokens; j++)
	{
		length = t[j].end-t[j].start;
		key_name = calloc(length+1, sizeof(char));
		memcpy(key_name, &configuration[t[j].start], length);

		//+++++++++++++++++++++++++++++++++++++++
		//search for operation
		if(strcmp(to_lower_case(key_name),"streams") == 0)
		{
			j++;

			if (t[j].type == JSMN_ARRAY)
			{
				//++++++++++++++++++++++++++++++++++
				// get end position of array streams
				array_streams_end_pos = t[j].end;
				while(t[j+1].start < array_streams_end_pos && j < number_of_tokens)
				{
					j++;
					object_stream_end_pos = t[j].end;

					while(t[j+1].start < object_stream_end_pos && j < number_of_tokens)
					{
						j++;

						if (t[j].type == JSMN_STRING)
						{
							//++++++++++++++++++++++++++++++++++
							// get object key name
							free(key_name);
							length = t[j].end-t[j].start;
							key_name = calloc(length+1, sizeof(char));
							memcpy(key_name,&configuration[t[j].start],length);	// get object key name

							//++++++++++++++++++++++++++++++++++
							// get twin id
							if (strcmp(to_lower_case(key_name),"id") == 0 )
							{
								j++;
								length = t[j].end-t[j].start;

								memset(id_temp,0x0,37);
								memcpy(id_temp,&configuration[t[j].start],min2_uint32_t(length,36));	// save value meta data id
							}
							//++++++++++++++++++++++++++++++++++
							// get name
							else if (strcmp(to_lower_case(key_name),"name") == 0 )
							{
								j++;
								length = t[j].end-t[j].start;

								memset(stream_name_temp,0x0,50);
								memcpy(stream_name_temp,&configuration[t[j].start],min2_uint32_t(length,49));

								if (strcmp(stream_name_temp,stream_name) == 0 )
								{
									strcpy(id,id_temp);
									free(key_name);
									free(t);
									return true;
								}
							}
						}
					}
				}
			}
		}
		free(key_name);
	}
	free(t);

	return false;
}
