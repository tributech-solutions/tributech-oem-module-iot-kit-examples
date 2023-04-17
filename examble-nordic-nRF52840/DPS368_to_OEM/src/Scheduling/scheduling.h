/*
 * dps368_scheduling.h
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include <zephyr.h>
#include <stdio.h>

extern struct k_work_q dps368_workqueue;
extern struct k_timer dps368_timer;
extern bool tmp_available;
extern bool psr_available;
// we cannot use time_t here since it is in sec and the OEM works with nanosec
extern uint64_t unix_timestamp;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize workqueue and timers and start both
void workqueue_init_and_start();

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// start unix timer
void start_unix_timer();

#endif /* SCHEDULING_H_ */