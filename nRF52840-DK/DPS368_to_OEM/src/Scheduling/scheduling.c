/*
 * dps368_scheduling.c
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

#include <zephyr.h>
#include <kernel.h>
#include <scheduling.h>
#include <dps368.h>
#include <stdio.h>
#include <uart_communication.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(scheduling, LOG_LEVEL_INF);

 #define SCHEDULER_PRIORITY 5
struct k_work_q dps368_workqueue;
struct k_timer dps368_timer;
struct k_work dps368_work_item;
bool tmp_available = false;
bool psr_available = false;
float last_tmp = 0;
float last_psr = 0;

/* CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE is configured in prj.conf */
K_THREAD_STACK_DEFINE(application_stack_area,
		      CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE);


// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// workitem function gather dps368 values and publish them
void trigger_dps368_value_gathering(struct k_work *work)
{
    bool ret = false;
    ret = dps368_measure_tmp_psr();
    if(ret != true)
    {
        LOG_ERR("Dps368 value aquisition failed!");
        return;
    }
    if(last_psr != data.psr_val)
    {
        if(&data.psr_val != NULL)
        {
            last_psr = data.psr_val;
            psr_available = true;
        }
        else
        {
            data.psr_val = last_psr;
            psr_available = true;
        }
    }
    if(last_tmp != data.tmp_val)
    {
        if(&data.tmp_val != NULL)
        {
            last_tmp = data.tmp_val;
            tmp_available = true;
        }
        else
        {
            data.tmp_val = last_tmp;
            tmp_available = true;
        }
    }
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize workqueue and timers and start both
void workqueue_init_and_start()
{
    // Start dps368 workqueue
    k_work_queue_start(&dps368_workqueue, application_stack_area,
                       K_THREAD_STACK_SIZEOF(application_stack_area), SCHEDULER_PRIORITY,
                       NULL);
    
    // Start dps368 function with a 1 minute periode
    k_timer_start(&dps368_timer, K_SECONDS(60), K_SECONDS(60));
    
    // Initialize dps368 workqueue item with associated function
    k_work_init(&dps368_work_item, trigger_dps368_value_gathering);
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// Dps368 timer capture handler
static void dps368_timer_handler(struct k_timer *dummy)
{
    k_work_submit_to_queue(&dps368_workqueue, &dps368_work_item);
}

K_TIMER_DEFINE(dps368_timer, dps368_timer_handler, NULL);

