#include "time_task.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
//#include <timeutil.h>
//#include <timeutil.h> //no such file or dir


#include "../devices/rv-3032-c7.h"
#include "../buffers/fifo_gen.h"
#include "../devices/display.h"
#include "display_task.h"

#include "../time/time_local.h"

#include "../mqtt/cSLIM_MQTT.h"

#include "gps_task.h"
LOG_MODULE_REGISTER(time_task, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 128

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

K_MSGQ_DEFINE(time_task_event_msgq, sizeof(time_task_event_msgq_data_item_t), 1, 32);

K_MSGQ_DEFINE(time_task_time_msgq, sizeof(time_task_time_msgq_data_item_t), 1, 128);
//static unsigned char    rtc_buffer[512];

//////////////////////////////////////////////////////////
//Private variables
//////////////////////////////////////////////////////////


time_t time_t_at_last_gps_correction; 

time_t gps_time_t_at_gps_correction;
uint32_t k_cycle_at_gps_update = 0;
time_t gps_time_t_at_time_update;

uint32_t minutes_to_next_gps_update;
time_t next_gps_wake_time;
//struct timeutil_sync_instant time_sync; 

const struct device *counter_dev;

////////////////////////////////////////////////////////////
//Private functions

int8_t missed_rtc_clkout_pulses;
void rtc_clkout_callback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
		uint32_t k_cycle = k_cycle_get_32();
		time_local_add_rtc_tick(&k_cycle);

}


void rtc_nint_callback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_INF("RTC nINT Callback at %" PRIu32 "\n", k_cycle_get_32());
}

//////////////////////////////////////////////////////////
//Public functions
//////////////////////////////////////////////////////////
void time_task_add_event(time_task_event ev)
{
	 while (k_msgq_put(&time_task_event_msgq, &ev, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&time_task_event_msgq);
        }
}

void time_task_add_event_with_timestamp(time_task_event ev, time_t ts_unix, uint32_t ts_msec, uint32_t k_cycle)
{
	time_task_event_msgq_data_item_t event_msg;
	event_msg.ev = ev; 
	event_msg.timestamp_unix = ts_unix;
	event_msg.timestamp_msec = ts_msec;
	event_msg.k_cycle = k_cycle;
	 while (k_msgq_put(&time_task_event_msgq, &event_msg, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&time_task_event_msgq);
        }
}

void time_task_add_time_msg(time_task_time_msgq_data_item_t* time_data)
{
	 while (k_msgq_put(&time_task_time_msgq, time_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&time_task_time_msgq);
        }
}
void time_task(void)
{
	LOG_INF("Is thread ID %x", (int) k_current_get());

	time_local_init();
	struct tm time;
	time.tm_hour = 0;
	time.tm_min = 0;
	time.tm_sec = 0;
	time.tm_mday = 1;
	time.tm_mon = 0;
	time.tm_year = 0;

	time_t local_time = mktime(&time);
	uint32_t k_cycle = k_cycle_get_32();
	time_local_set_date_and_time(&local_time, &k_cycle);
	time_local_set_date_and_time_at_gps_correction(&local_time);

	minutes_to_next_gps_update = 0;
	next_gps_wake_time = 0;
	time_local_set_rtc_estimated_frequency(1.0);


	k_sleep(K_SECONDS(5));
	LOG_INF("Wake for RTC init");

	//Initialize RTC and set clockout
	rv3032_init();
	rv3032_clkout_disable();
	rv3032_set_clockout_frequency_lf(F_1HZ);
	rv3032_clkout_enable();
	k_sleep(K_MSEC(500));
	rv3032_register_callbacks(rtc_clkout_callback, rtc_nint_callback);


	k_msgq_purge(&time_task_time_msgq);
	
	gps_task_add_event(GPS_WAKEUP_REQUEST);
	time_task_state state = GPS_WAKEUP;
	time_task_event_msgq_data_item_t time_task_event_msgq_item; 
	time_task_time_msgq_data_item_t time_data;

	time_t time_t_local_time;
	time_t elapsed_time_s;
	struct tm time_tm_local_temp;
	uint32_t microseconds_local_at_gps_correction;
	struct tm gps_time_tm_at_gps_correction;
	struct tm time_tm_local_time_at_gps_correction_tm;
	int16_t offset_seconds;
	struct tm wake_time;

	int status;
	while(1)
	{
	
		switch (state)
		{
		case IDLE2:
			break;
		case GPS_WAKEUP:
			
			status = k_msgq_get(&time_task_event_msgq, &time_task_event_msgq_item, K_MSEC(1100));
			if (status == 0)
			{
				if (time_task_event_msgq_item.ev == GPS_UPDATE_ACCEPTED)
				{
					state = UPDATE_TIME_DATA_GPS;
					LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
				}
			}
			
			break;
		case UPDATE_TIME_DATA_GPS:
				if (k_msgq_get(&time_task_time_msgq, &time_data, K_SECONDS(2)) == 0)
				{
					LOG_INF("Got msg from GPS task");
					
					//Got time data from GPS task, update RTC
					k_sleep(K_MSEC(110)); //Ensure GPS tp is low
					//Set up RTC to wait for clock pulse  (rising edge)
					rv3032_enable_evi_clock_reset();  	
					//rv3032_clkout_disable();
					rv3032_set_date_and_time(&(time_data.tm_task_time));  //Set to gps time

					rv3032_enable_evi_clock_reset();  


					gps_task_add_event(READY_FOR_GPS_SYNC);
					k_msgq_purge(&time_task_time_msgq);  
					gps_time_t_at_time_update = mktime(&(time_data.tm_task_time));
					k_cycle_at_gps_update = k_cycle_get_32();
					state = WAIT_FOR_GPS_SYNC;
					LOG_INF("Change state: WAIT_FOR_GPS_SYNC");
					
					

				}	
				else
				{
					LOG_DBG("waiting for time data");
				}	
			break;
		case WAIT_FOR_GPS_SYNC:
			status = k_msgq_get(&time_task_time_msgq, &time_data, K_MSEC(1200));
			if (status == 0)
			{

					//Make sure GPS timepulse did not occur before RTC was ready
					if(time_data.k_cycle < k_cycle_at_gps_update)
					{
						LOG_ERR("GPS Timepulse occured before RTC was ready, restart");
						state = UPDATE_TIME_DATA_GPS;
						LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
						break;
					}
					//Get local time at GPS timepulse without correction
					microseconds_local_at_gps_correction = time_data.timestamp_msec;

					gps_time_t_at_gps_correction = mktime(&(time_data.tm_task_time));
					gps_time_tm_at_gps_correction = time_data.tm_task_time;

					//make sure time is set correct
					uint32_t time_since_time_update = difftime(gps_time_t_at_gps_correction, gps_time_t_at_time_update);
					if(time_since_time_update != 0)
					{
						LOG_ERR("Time update failed, timediff is %d, restart", time_since_time_update);
						state = UPDATE_TIME_DATA_GPS;
						LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
						break;
					}

					//check that RTC time and GPS time is equal
					struct tm rtc_time_tm = rv3032_get_date_and_time();
					time_t rtc_time_t = mktime(&rtc_time_tm);
					int64_t gps_rtc_time_diff = (int) difftime(rtc_time_t, gps_time_t_at_gps_correction);
					if(gps_rtc_time_diff != 0)
					{
						LOG_ERR("RTC and GPS time not equal, restart time sync");
						state = UPDATE_TIME_DATA_GPS;
						LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
						break;
					} 


					//Success
					time_t new_local_time_t = gps_time_t_at_gps_correction + 1; 
					time_local_set_date_and_time(&new_local_time_t, &(time_data.k_cycle)); //set local time to gps time
					
					//Get elapsed time since last successful update
					elapsed_time_s = (int) difftime(gps_time_t_at_gps_correction, time_t_at_last_gps_correction);
					time_t_at_last_gps_correction = gps_time_t_at_gps_correction;

					if(minutes_to_next_gps_update == 0 || minutes_to_next_gps_update == 1) //First update, RTC is set 
					{
						LOG_INF("First update, discarding time correction");
						
						minutes_to_next_gps_update = 2; //Has to be set to minimum two for interval to increase at 0 offset
						next_gps_wake_time = gps_time_t_at_gps_correction + 5; //wait five seconds
						time_local_set_rtc_estimated_frequency(1);
						gps_task_add_event(TIME_UPDATE_COMPLETE);
						state = ON_LOCAL_TIME;
						LOG_INF("Change state: ON_LOCAL_TIME");
						break;
					}	

					//Log local time, use LOG_ERR for visibility in log
					time_tm_local_time_at_gps_correction_tm = *localtime(&(time_data.timestamp_unix)); //get local time at GPS timepulse
					LOG_ERR("Local time is %d-%d-%d  %d:%d:%d.%6d",time_tm_local_time_at_gps_correction_tm.tm_year, time_tm_local_time_at_gps_correction_tm.tm_mon, time_tm_local_time_at_gps_correction_tm.tm_mday, time_tm_local_time_at_gps_correction_tm.tm_hour, time_tm_local_time_at_gps_correction_tm.tm_min, time_tm_local_time_at_gps_correction_tm.tm_sec, microseconds_local_at_gps_correction);
					LOG_ERR("GPS time is %d-%d-%d  %d:%d:%d", gps_time_tm_at_gps_correction.tm_year, gps_time_tm_at_gps_correction.tm_mon, gps_time_tm_at_gps_correction.tm_mday, gps_time_tm_at_gps_correction.tm_hour, gps_time_tm_at_gps_correction.tm_min, gps_time_tm_at_gps_correction.tm_sec);
								

								
					//find total offset
					offset_seconds = gps_time_t_at_gps_correction - time_data.timestamp_unix;  //GPS time - local time
					
					//Update RTC estimated frequency (with filter)
					long double frequency_rtc_since_last_update = 1;

					LOG_INF("Offset seconds are: %d", microseconds_local_at_gps_correction);
					int32_t microseconds_local_signed = 0; 

					if(offset_seconds >= 1) //Local time behind gps time
					{
						//if local time is 1.999500 while gps time is 2.000000, the offset is 0 seconds and 500us (offset is always positive)
						offset_seconds--;
						microseconds_local_at_gps_correction = USEC_PER_SEC - microseconds_local_at_gps_correction ;
						microseconds_local_signed = -microseconds_local_at_gps_correction;


						LOG_ERR("Localtime is behind with %d seconds and %" PRIu32 " microseconds", offset_seconds, microseconds_local_at_gps_correction);
						//microseconds_local_at_gps_correction += USEC_PER_SEC * abs(offset_seconds);
						frequency_rtc_since_last_update = ( (double)(USEC_PER_SEC * elapsed_time_s)) / ( (double)USEC_PER_SEC * elapsed_time_s + microseconds_local_at_gps_correction);


					}
					else //Loval time is ahead of
					{
						LOG_ERR("Localtime is ahead with %d seconds and  %" PRIu32 " microseconds",  offset_seconds, microseconds_local_at_gps_correction);
						//microseconds_local_at_gps_correction += USEC_PER_SEC * abs(offset_seconds);
						if(microseconds_local_at_gps_correction > 500000) //assume wrong second, can be result of UTC correction at startup
						{
							uint32_t microseconds_local_at_gps_correction_temp = USEC_PER_SEC - microseconds_local_at_gps_correction;
							frequency_rtc_since_last_update = ( (long double)(USEC_PER_SEC * elapsed_time_s)) / ( (long double)USEC_PER_SEC * elapsed_time_s - microseconds_local_at_gps_correction_temp);
						}
						else
						{
							frequency_rtc_since_last_update = ( (long double)(USEC_PER_SEC * elapsed_time_s)) / ( (long double)USEC_PER_SEC * elapsed_time_s - microseconds_local_at_gps_correction); //this is how it should be if seconds are always right
						}
						microseconds_local_signed = microseconds_local_at_gps_correction;
					}
					LOG_INF("Estimated frequency before correction: %d nHz", (int) (time_local_get_rtc_estimated_frequency()*1000000000));

					if(microseconds_local_at_gps_correction < 500000) //do not update if second is wrong (for instance because of UTC correction at startup)
					{
						time_local_set_rtc_estimated_frequency( time_local_get_rtc_estimated_frequency() * 0.7 + (1-0.7)*frequency_rtc_since_last_update*time_local_get_rtc_estimated_frequency());
					}
					

					LOG_INF("Estimated frequency after correction: %d nHz", (int) (time_local_get_rtc_estimated_frequency()*1000000000));

					time_local_set_date_and_time_at_gps_correction(&gps_time_t_at_gps_correction);

					//calculate time til next correction
					if ((microseconds_local_at_gps_correction > 500 ))//&& offset_seconds <= 0) || (offset_seconds != 0 && offset_seconds != 1) )//Did not meet constraints, reduce correction time
					{
						minutes_to_next_gps_update /= 2;
						
						LOG_ERR("Did not meet 500us constraint");
							
					}
					else if(microseconds_local_at_gps_correction != 0) //Correction is made, use to calculate time till next GPS-update
					{
						//minutes_to_next_gps_update = 250 * minutes_to_next_gps_update / microseconds_local_at_gps_correction; //divide by two to be safe
						minutes_to_next_gps_update = 0.5*minutes_to_next_gps_update + (1-0.5)*( 250 * minutes_to_next_gps_update / microseconds_local_at_gps_correction); //divide by two to be safe
						LOG_INF("");
					}
					else //No correction needed, increase time to next update
					{
						//minutes_to_next_gps_update *= 1.5;
						//LOG_INF("increasing update interval by x1.5");
						minutes_to_next_gps_update *= 1.5; //aggressive for reducing time of testing
						LOG_INF("increasing update interval by x5");
					}

					if(minutes_to_next_gps_update < 2)
					{
						minutes_to_next_gps_update = 2;
					}
					else if(minutes_to_next_gps_update > 1440) //24 hours, measure rtc temp more often and reduce interval/wake up if temperature change is large
					{
						minutes_to_next_gps_update = 1440;

					}

					//

					//Log RTC drift

					LOG_DBG("Seconds passed: %d", (uint32_t) elapsed_time_s);
					LOG_INF("Clock drifted %d us in %d:%d:%d",(int32_t) microseconds_local_signed, (uint8_t) (elapsed_time_s/3600), (uint8_t) ((elapsed_time_s%3600)/60), (uint8_t) (elapsed_time_s % 60));
					static unsigned char    display_buffer[512];
					
					sprintf(display_buffer,"Clk drift:");
					display_put_string(5, 40, display_buffer,font_medium);
					sprintf(display_buffer,"                                     ");
					display_put_string(5, 60, display_buffer,font_medium);
					sprintf(display_buffer,"%ds %d us in %d:%d:%d", (uint32_t) offset_seconds, (int32_t) microseconds_local_signed, (uint8_t) (elapsed_time_s/3600), (uint8_t) (elapsed_time_s/60)%60, (uint8_t) (elapsed_time_s % 60));
					display_put_string(5, 60, display_buffer,font_medium);

					cSLIM_mqtt_data_publish(display_buffer,(size_t) 25);
					


	
						
					//minutes_to_next_gps_update = 2;  //TODO Remove after TESTING

					next_gps_wake_time = gps_time_t_at_gps_correction + minutes_to_next_gps_update*60;
					wake_time = *localtime(&next_gps_wake_time);

					sprintf(display_buffer,"                                     ");
					display_put_string(5, 100, display_buffer,font_small);
					sprintf(display_buffer,"Next wake time: %d:%d:%d", wake_time.tm_hour, wake_time.tm_min, wake_time.tm_sec);
					display_put_string(5, 100, display_buffer,font_small);
					request_display_update();

					gps_task_add_event(TIME_UPDATE_COMPLETE);
					state = ON_LOCAL_TIME;
					LOG_INF("Change state: ON_LOCAL_TIME");
					LOG_INF("On local time, waiting %d minutes or for temperature change before restarting GNSS ", minutes_to_next_gps_update);
				
				
				break; //If other message, wait for new
			}
			if(k_msgq_get(&time_task_event_msgq, &time_task_event_msgq_item, K_MSEC(1200))==0)
				{
					if (time_task_event_msgq_item.ev == GPS_UPDATE_REJECTED)
					{
						//Rejected update, go back
						state = UPDATE_TIME_DATA_GPS;
						LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
						break;
					}
				}
			else
			{
				//No message, restart
				gps_task_add_event(GPS_WAKEUP_REQUEST);
				state = UPDATE_TIME_DATA_GPS;
				LOG_INF("Change state: UPDATE_TIME_DATA_GPS");
				break;
			}	
			break;
		
		case ON_LOCAL_TIME:
			
			
			
			time_local_get_date_and_time_t(&time_t_local_time);
			time_tm_local_temp = *localtime(&time_t_local_time);
			LOG_INF("Wakeup in %d seconds", (int) (next_gps_wake_time - time_t_local_time));
			status = k_msgq_get(&time_task_event_msgq, &time_task_event_msgq_item, K_SECONDS(next_gps_wake_time - time_t_local_time));
			if (status == 0)
			{
				if (time_task_event_msgq_item.ev == RTC_TEMPERATURE_CHANGED)
				{
					time_local_get_date_and_time_t(&time_t_local_time);
					next_gps_wake_time = time_t_local_time + 60; //wait one minute to ensure some time between updates 
				}
			}
			else if(status == -EAGAIN)
			{
				time_local_get_date_and_time_t(&time_t_local_time);
				time_tm_local_temp = *localtime(&time_t_local_time);
				LOG_INF("Local time is \t%d:%d:%d Date %d-%d-%d", time_tm_local_temp.tm_hour, time_tm_local_temp.tm_min, time_tm_local_temp.tm_sec, time_tm_local_temp.tm_year + 1900 , time_tm_local_temp.tm_mon + 1, time_tm_local_temp.tm_mday);
				time_tm_local_temp = rv3032_get_date_and_time();
				LOG_INF("Wake, rtc time is \t%d:%d:%d Date %d-%d-%d", time_tm_local_temp.tm_hour, time_tm_local_temp.tm_min, time_tm_local_temp.tm_sec, time_tm_local_temp.tm_year + 1900 , time_tm_local_temp.tm_mon + 1, time_tm_local_temp.tm_mday);
				gps_task_add_event(GPS_WAKEUP_REQUEST);
				state = GPS_WAKEUP;
				LOG_INF("Change state: GPS_WAKEUP");
			}			
			break;
		
		default:
			break;
		}
		
		//TODO Make state machine
		
	
	}
	
}