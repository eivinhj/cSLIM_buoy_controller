#include "tbr_sync_task.h"

#include "display_task.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>

#include "devices/tbr.h"
#include "../time/time_local.h"
#include "../devices/display.h"
LOG_MODULE_REGISTER(tbr_sync_task, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 128

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

K_FIFO_DEFINE(tbr_task_fifo);

//static unsigned char    tbr_buffer[512];

int8_t tbr_sync_offset_seconds;
uint32_t tbr_sync_offset_us;

void tbr_sync_set_offset(int8_t offset_sec, uint32_t offset_usec)
{
	tbr_sync_offset_seconds = offset_sec;
	tbr_sync_offset_us = offset_usec;
}

void tbr_sync_get_offset(int8_t* offset_sec, uint32_t* offset_usec)
{
	*offset_sec = tbr_sync_offset_seconds;
	*offset_usec = tbr_sync_offset_us;
}


void tbr_sync_task(void)
{
	LOG_INF("Is thread ID %x", (int) k_current_get());
	k_sleep(K_SECONDS(5));
	LOG_INF("TBR Task starting");
	//Initialize RTC and set clockout
	bool tbr_found = false;
	tbr_init();

	LOG_INF("TBR Task init complete");
	struct tm time_local_tm;
	time_t time_local;
	uint32_t us;
	uint32_t k_cycle = k_cycle_get_32();
	tbr_sync_set_offset(0,0);

	k_sleep(K_MSEC(2));

	int init_retry = 0; 
	do {
        tbr_found = tbr_send_cmd(cmd_sn_req, (time_t) 0);
        if (++init_retry > 250) {
            LOG_INF("TBR no response\n");
            break;
        }
        k_sleep(K_MSEC(2));
    } while (!tbr_found);

    display_put_string(6, 3 * 12 + 4, "TBR:         ", font_medium);
    if(tbr_found) {
        unsigned char display_buffer[8];
        sprintf((char *) display_buffer, "%d", get_tbr_serial_number());
        display_put_string(6 + 3*12 + 2, 3 * 12 + 4, display_buffer, font_medium);
    }
    else {

        display_put_string(6 + 3*12 + 2, 3 * 12 + 4, "Missing!", font_medium);
    }
    //display_update();	
	request_display_update();
	
	int valid_timestamp = false;

	while(1)
	{
		k_sleep(K_MSEC(8500));
		valid_timestamp = true;

		LOG_DBG("Wake");
		if(tbr_found)
		{
			k_cycle = k_cycle_get_32();
			if(get_precise_local_date_and_time_tm(&time_local_tm, &us, &k_cycle) == 0)
			{
				
				while( (us < 950000) || ((time_local_tm.tm_sec % 10) != 9) ) //wait for correct time within 15ms, takes ~1ms to send message, rest is coordinated
				{
					//LOG_DBG("in loop %d.%d",time_local_tm.tm_sec, us );
					k_sleep(K_MSEC(25));
					if (time_local_tm.tm_sec > 0 && time_local_tm.tm_sec < 8)
					{
						LOG_INF("Missed TBR pulse deadline");
						break;
						valid_timestamp = false;
					}
					k_cycle = k_cycle_get_32();
					get_precise_local_date_and_time_tm(&time_local_tm, &us, &k_cycle);
				}
				if(valid_timestamp)
				{
					time_local = mktime(&time_local_tm);
					LOG_DBG("Send TBR cmd");
					LOG_DBG("Local time is %d-%d-%d %d:%d:%d.%d", time_local_tm.tm_year, time_local_tm.tm_mon, time_local_tm.tm_mday, time_local_tm.tm_hour, time_local_tm.tm_min, time_local_tm.tm_sec, us);
					tbr_send_cmd(cmd_advance_sync, time_local);
				}
				
			}
			else
			{
				LOG_INF("Could not get precise date and time");
			}	
		}
		else
		{
			k_sleep(K_SECONDS(30));
			init_retry = 0; 
			do {
				tbr_found = tbr_send_cmd(cmd_sn_req, (time_t) 0);
				if (++init_retry > 250) {
					LOG_INF("TBR no response\n");
					break;
				}
				k_sleep(K_MSEC(2));
			} while (!tbr_found);

			display_put_string(6, 3 * 12 + 4, "TBR:         ", font_medium);
			if(tbr_found) {
				unsigned char display_buffer[8];
				sprintf((char *) display_buffer, "Active");
				display_put_string(6 + 3*12 + 2, 3 * 12 + 4, display_buffer, font_medium);
			}
			//display_update();	
			request_display_update();

		}
		
		
	}
}