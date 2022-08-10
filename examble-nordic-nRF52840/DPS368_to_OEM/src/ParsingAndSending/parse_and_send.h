/*
 * parse_and_send.h
 *
 *  Created on: 28 May 2022
 *      Author: A.Ascher
 */

#ifndef PARSE_AND_SEND_H_
#define PARSE_AND_SEND_H_

#include <zephyr.h>

extern uint32_t transaction_nr;
extern struct k_sem sending_sem;
extern char valuemetadataid_tmp[37];
extern char valuemetadataid_psr[37];

//send temperature value to oem in provide values sleeve
void send_tmp_value_to_oem(void);

//send pressure value to oem in provide values sleeve
void send_psr_value_to_oem(void);

// parse return json of oem.
int parse_oem_response(void);

// send both temperature and pressure to the oem in one package
void send_tmp_and_psr_value_to_oem(void);

// init sending semaphore
void init_send_semaphore(void);

#endif /* PARSE_AND_SEND_H_ */