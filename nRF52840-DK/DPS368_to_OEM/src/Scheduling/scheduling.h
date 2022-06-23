/*
 * dps368_scheduling.h
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include <zephyr.h>

extern struct k_work_q dps368_workqueue;
extern struct k_timer dps368_timer;
extern char valuemetadataid_tmp[100];
extern char valuemetadataid_psr[100];
extern bool tmp_available;
extern bool psr_available;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize workqueue and timers and start both
void workqueue_init_and_start();


#endif /* SCHEDULING_H_ */