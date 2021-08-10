#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "gps_task.h"
#include "time_task.h"
#include "cslim_status_task.h"
#include "display_task.h"
#include "../devices/display.h"
#include "../time/time_manager.h"
#include "../devices/ublox_gps.h"
#include "../buffers/fifo_gen.h"
#include "../time/time_local.h"

LOG_MODULE_REGISTER(gps_task, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 128

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

#define INIT_RETRIES 250

K_FIFO_DEFINE(gps_task_fifo);
K_MSGQ_DEFINE(gps_task_event_msgq, sizeof(gps_task_event_msgq_data_item_t), 1, 8);

K_MSGQ_DEFINE(gps_task_nav_msgq, sizeof(nav_data_t), 1, 64);
K_MSGQ_DEFINE(gps_task_time_msgq, sizeof(time_data_t), 1, 32);
K_MSGQ_DEFINE(gps_task_sat_msgq, sizeof(sat_data_t), 1, 8);
K_MSGQ_DEFINE(gps_task_ack_msgq, sizeof(ack_data_t), 1, 4);


time_data_t time_data;
gps_task_event_msgq_data_item_t gps_task_ISR_msgq_item;

int8_t last_gps_poll_seconds = -1;
uint8_t number_of_success_polls = 0;


uint32_t last_gps_timepulse = 0; 

//////////////////////////////////////////////////////////
//Public functions
//////////////////////////////////////////////////////////

void gps_task_add_event(gps_task_event ev)
{
	 while (k_msgq_put(&gps_task_event_msgq, &ev, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_event_msgq);
        }
}


//////////////////////////////////////////////////////////
//Private functions
//////////////////////////////////////////////////////////
void gps_task_add_time_msg(time_data_t* time_data)
{
	 while (k_msgq_put(&gps_task_time_msgq, time_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_time_msgq);
        }
}

void gps_task_add_nav_msg(nav_data_t* nav_data)
{
	 while (k_msgq_put(&gps_task_nav_msgq, nav_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_nav_msgq);
        }
}

void gps_task_add_sat_msg(sat_data_t* sat_data)
{
	 while (k_msgq_put(&gps_task_sat_msgq, sat_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_sat_msgq);
        }
}

void gps_task_add_ack_msg(ack_data_t* ack_data)
{
	 while (k_msgq_put(&gps_task_ack_msgq, ack_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_ack_msgq);
        }
}

void gps_timepulse_callback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	uint32_t k_cycle = k_cycle_get_32();
	time_t ts_unix;
	uint32_t ts_msec; 

	get_precise_local_date_and_time_t(&ts_unix, &ts_msec, &k_cycle);
	gps_task_ISR_msgq_item.timestamp_unix = ts_unix;
	gps_task_ISR_msgq_item.timestamp_msec = ts_msec;
	gps_task_ISR_msgq_item.k_cycle		  = k_cycle;
	//LOG_ERR("GPS Timepulse Callback ");
	gps_task_ISR_msgq_item.ev = TIMEPULSE_RECEIVED; 
	last_gps_timepulse = k_cycle;

	//put in buffer
	while (k_msgq_put(&gps_task_event_msgq, &gps_task_ISR_msgq_item, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&gps_task_event_msgq);
        }
}

void handle_gps_message(ubx_msg_t* recv_msg){
        if (recv_msg->class == NAV && recv_msg->id == PVT) {
			LOG_DBG("NAV msg");
			nav_data_t nav_data; 
            update_nav_data(recv_msg, &nav_data);
			//send to queue
			gps_task_add_nav_msg(&nav_data);

        }
        else if (recv_msg->class == NAV && recv_msg->id == SAT) {
			LOG_DBG("SAT msg");
			sat_data_t sat_data;
			update_sat_data(recv_msg, &sat_data);
			//send to queue
			gps_task_add_sat_msg(&sat_data);
        }
        else if (recv_msg->class == TIM && recv_msg->id == MSG) {
			LOG_DBG("TIM MSG");
			time_data_t tim_data;
        	parse_tim_data(recv_msg, &tim_data);
			time_manager_update_utc_and_unix_time(&tim_data);  //TODO
			//send to queue
			gps_task_add_time_msg(&tim_data);
        }
        else if (recv_msg->class == ACK && recv_msg->id == 0x01) {
            LOG_DBG( "ACK msg\n");
			ack_data_t ack_data;
            update_ack_data(recv_msg, &ack_data, true);
			//send to queue
			gps_task_add_ack_msg(&ack_data);
        }
        else if (recv_msg->class == ACK && recv_msg->id == 0x00) {
            LOG_DBG( "NAK msg\n");
			ack_data_t ack_data;
            update_ack_data(recv_msg, &ack_data, false);
			//send to queue
			gps_task_add_ack_msg(&ack_data);
		}
        else {
            LOG_ERR( "gps_read_spi_data: unknown msg, class 0x%2x, id: 0x%2x\n", recv_msg->class, recv_msg->id);
            k_sleep(K_MSEC(2));
        }
    
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//GPS TASK
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
void gps_task(void)
{
	LOG_INF("Is thread ID %x", (int) k_current_get());
	uint8_t init_retry=0;
	bool	temp_init_flag=false;
	k_sleep(K_SECONDS(5));


 	display_put_string(6, 4*12+5,"GPS:Search",font_medium);
	request_display_update();
    init_retry = 0;
    do {
        LOG_INF("GPS init called\n");
        temp_init_flag = gps_init(gps_timepulse_callback);		
        if(init_retry++ > INIT_RETRIES) {
            LOG_ERR("GPS INIT FAILED\n\n");
        }
        if (!temp_init_flag) {
            LOG_INF("GPS: init failed. Tries again\n");
        }
		k_sleep(K_MSEC(1000));
    } while (!temp_init_flag);
    LOG_INF("GPS Init DONE\n\n");

  
	display_put_string(1, 4*12+5, "              ", font_medium);
	display_put_string(1, 5*12+5, "              ", font_medium);
	display_put_string(1, 6*12+5, "              ", font_medium);
    display_put_string(6, 4*12+5, "GPS: Init OK", font_medium);

    //display_update();
	request_display_update();


	time_t time_t_last_wake_time;
	time_t_last_wake_time = 0;

	gps_task_state state = IDLE;
	time_task_time_msgq_data_item_t time_task_data;
	//gps_enable_pps();

	while(1)
	{
		int status;
		gps_task_event_msgq_data_item_t gps_task_event_msgq_item; 
		nav_data_t nav_data;
		ubx_msg_t   recv_msg;

		switch(state)
		{
			case IDLE:
				status = k_msgq_get(&gps_task_event_msgq, &gps_task_event_msgq_item, K_FOREVER);
				if(gps_task_event_msgq_item.ev == GPS_WAKEUP_REQUEST)
				{	
					time_task_add_event(GPS_UPDATE_ACCEPTED); 
					
					//TODO Use warm or hot start if it is not too long since last fix, cold start otherwise
					if(time_t_last_wake_time == 0) //First wake
					{
						//gps_send_gnss_wakeup_cold_start();
						LOG_INF("First fix, gps already awake");
					}
					else
					{
						struct tm local_time;
						time_local_get_date_and_time_tm(&local_time);
					
						time_t time_t_now 	= mktime(&local_time);
						
						double diff_seconds = difftime(time_t_now, time_t_last_wake_time);
						if(diff_seconds < 4*60*60)	//Ephemeris valid for four hours
						{
							gps_send_gnss_wakeup_hot_start();
							LOG_INF("Hot start");
						}
						else	//approximate information of position and time
						{
							gps_send_gnss_wakeup_warm_start();
							LOG_INF("Warm start");
						} 
					}
					gps_enable_pps();
					state = UPDATE_POS;
					LOG_INF("Change state: UPDATE_POS");
				}
				break;
			case UPDATE_POS:
				status = k_msgq_get(&gps_task_event_msgq, &gps_task_event_msgq_item, K_MINUTES(15)); //K_MSEC(500)
				if (status == 0)
				{
					if(gps_task_event_msgq_item.ev == TIMEPULSE_RECEIVED)
					{
						//LOG_INF("Receive gps timepulse");
						status = gps_read_spi_data(&recv_msg);
						LOG_INF("read_gps_spi_data complete");
						//k_sleep(K_MSEC(100)); //For testing
						if(status == true) //Reading message is successfull
						{
							handle_gps_message(&recv_msg);
							if(k_msgq_get(&gps_task_nav_msgq, &nav_data, K_NO_WAIT) != ENOMSG)
							{
								//Got new time data 
								LOG_INF("New pos data: Long:%d, Lat: %d ", nav_data.longitude, nav_data.latitude);
								
								if(nav_data.longitude != 0 && nav_data.latitude != 0)
								{
									//Send to cslim status task
									cslim_status_task_add_nav_message(&nav_data);

									state = POLL_TIME_DATA;
									LOG_INF("Change state: POLL_TIME_DATA");

									//gps_poll_tim_status();
									gps_poll_nav_data();
									break;
								}
								
								
							}
						}
						gps_poll_nav_data();
						k_msgq_purge(&gps_task_event_msgq);
					}
				
				}
				else
				{
					LOG_INF("No timepulse arrived for 5 minutes, change state to GNSS wakeup");
					state = IDLE;
					LOG_INF("Change state: IDLE");
					gps_task_add_event(GPS_WAKEUP_REQUEST);

				}
				break;
			case POLL_TIME_DATA:
				//gps_poll_tim_status();
				LOG_DBG("POLL_TIME_DATA: Wait for timepulse");
				status = k_msgq_get(&gps_task_event_msgq, &gps_task_event_msgq_item, K_MINUTES(15));
				LOG_DBG("POLL_TIME_DATA: Timeout or message");
				k_sleep(K_MSEC(100));
				if (status == 0)
				{
					if(gps_task_event_msgq_item.ev == TIMEPULSE_RECEIVED)
					{	
						LOG_DBG("POLL_TIME_DATA: Timepulse received");
						
						
						status = gps_read_spi_data(&recv_msg);

						if(status == true) //Reading message is successfull
						{
							handle_gps_message(&recv_msg);
							//if(k_msgq_get(&gps_task_time_msgq, &time_data, K_NO_WAIT) != ENOMSG)
							if(k_msgq_get(&gps_task_nav_msgq, &nav_data, K_NO_WAIT) != ENOMSG)
							{
								//Got new time data 
								LOG_ERR("New time data: %d-%d-%d, %d:%d:%d",nav_data.year, nav_data.month, nav_data.day, nav_data.hour, nav_data.min, nav_data.sec);

							//README: Discovered problem with tim-tp messages giving varying seconds x, then x+1, then x+3 or x, x, x+2 etc. 
							//Does not seem to be a problem with nav-pvt messages, so I am using this instead.
							//Not sure if the message from the GPS module itself is wrong or it is read in a problematic way.

							/*	if( !((abs(time_data.quantisation_error) < 1000000) && ((time_data.flags & 0b011) == 0) && (time_data.ref_info == 0) && (time_data.tow_sub_ms == 0)) )
								{
									LOG_ERR("Time accuracy too low");
									gps_poll_tim_status();
									break;
								}*/

								//TODO check that time data is not outdated and send to time_task
								
							/*	time_task_data.timestamp_unix = gps_task_event_msgq_item.timestamp_unix;
								time_task_data.timestamp_msec = gps_task_event_msgq_item.timestamp_msec;
								
  								time_task_data.tm_task_time.tm_sec 	=  time_data.sec;
  								time_task_data.tm_task_time.tm_min	=  time_data.min;
  								time_task_data.tm_task_time.tm_hour	=  time_data.hour;
  								time_task_data.tm_task_time.tm_mday	=  time_data.day;
  								time_task_data.tm_task_time.tm_mon	=  time_data.month -1;
  								time_task_data.tm_task_time.tm_year	=  time_data.year - 1900;
  								time_task_data.tm_task_time.tm_isdst=  -1;
								time_task_data.k_cycle = gps_task_event_msgq_item.k_cycle;
								*/

								time_task_data.timestamp_unix = gps_task_event_msgq_item.timestamp_unix;
								time_task_data.timestamp_msec = gps_task_event_msgq_item.timestamp_msec;
								
  								time_task_data.tm_task_time.tm_sec 	=  nav_data.sec + 1;  //for next time pulse
  								time_task_data.tm_task_time.tm_min	=  nav_data.min;
  								time_task_data.tm_task_time.tm_hour	=  nav_data.hour;
  								time_task_data.tm_task_time.tm_mday	=  nav_data.day;
  								time_task_data.tm_task_time.tm_mon	=  nav_data.month -1;
  								time_task_data.tm_task_time.tm_year	=  nav_data.year - 1900;
  								time_task_data.tm_task_time.tm_isdst=  -1;
								time_task_data.k_cycle = gps_task_event_msgq_item.k_cycle;

								//Send to timetask
								if(last_gps_timepulse != gps_task_event_msgq_item.k_cycle)
								{
									LOG_ERR("GPS Timepulse occured before message was processed");
									break;
								}
								time_task_add_time_msg(&time_task_data);
								k_msgq_purge(&gps_task_event_msgq);
								state = MAKE_CORRECTION;
								LOG_INF("Change state: MAKE_CORRECTION");
								//gps_poll_tim_status();
								break;
							}
						}
						else
						{
							LOG_ERR("Could not poll gps message, retry");
							//gps_poll_tim_status();
							gps_poll_nav_data();
							number_of_success_polls = 0;
							last_gps_poll_seconds = -1;
							k_msgq_purge(&gps_task_event_msgq);
							k_sleep(K_MSEC(1));
						}
						
					}
					else if(gps_task_event_msgq_item.ev == TIME_UPDATE_COMPLETE)	//RTC updated, wait for timepulse
					{
						//success
						time_task_add_event_with_timestamp(GPS_TIME_SYNC, gps_task_event_msgq_item.timestamp_unix, gps_task_event_msgq_item.timestamp_msec, gps_task_event_msgq_item.k_cycle);
						number_of_success_polls = 0;
						last_gps_poll_seconds = -1;
						state = POST_CORRECTION;
						LOG_INF("Change state: POST_CORRECTION");
						break;

					}
					
					
				}
				else
				{
					LOG_INF("No timepulse arrived for 5 minutes, change state to GNSS wakeup");
					state = IDLE;
					LOG_INF("Change state: IDLE");
					gps_task_add_event(GPS_WAKEUP_REQUEST);

				}
				break;
			case MAKE_CORRECTION:

				//TODO Wait for acknowledge of correction
				//Go back to poll time data if timepulse arrives before correction is made or timepulse does not arrive within 1 second.
				LOG_INF("Waiting for correction message");
				status = k_msgq_get(&gps_task_event_msgq, &gps_task_event_msgq_item, K_MINUTES(15));
				if (status == 0)
				{
					if(gps_task_event_msgq_item.ev == TIMEPULSE_RECEIVED)	//Timepulse received before time was corrected -> time is outdated
					{
						LOG_ERR("Timepulse received before acceptance");
						time_task_add_event(GPS_UPDATE_REJECTED);
						//Go back to fetch new time
						state = POLL_TIME_DATA;
						LOG_INF("Change state: POLL_TIME_DATA");
					}
					else if(gps_task_event_msgq_item.ev == READY_FOR_GPS_SYNC)	//RTC updated, wait for timepulse
					{
						LOG_INF("Ready for GPS sync");
						//Wait for next timepulse and make sure it is within one second
						status = k_msgq_get(&gps_task_event_msgq, &gps_task_event_msgq_item, K_MSEC(1000));
						if (status == 0)
						{
							if(gps_task_event_msgq_item.ev == TIMEPULSE_RECEIVED)
							{
								//success
								//time_task_add_event_with_timestamp(GPS_TIME_SYNC, gps_task_event_msgq_item.timestamp_unix, gps_task_event_msgq_item.timestamp_msec, gps_task_event_msgq_item.k_cycle);
								//state = POST_CORRECTION;
								//LOG_INF("Change state: POST_CORRECTION");

								time_task_data.k_cycle = gps_task_event_msgq_item.k_cycle;
								//Send to timetask
								if(last_gps_timepulse != gps_task_event_msgq_item.k_cycle)
								{
									LOG_ERR("GPS Timepulse occured before message was processed");
									time_task_add_event(GPS_UPDATE_REJECTED);
									state = POLL_TIME_DATA;
									LOG_INF("Change state: POLL_TIME_DATA");
									break;
								}
								time_task_add_time_msg(&time_task_data);
								break;
							}

						}
						//fail (no), go back
						LOG_ERR("Timepulse to late, discarding");
						time_task_add_event(GPS_UPDATE_REJECTED);
						state = POLL_TIME_DATA;
						LOG_INF("Change state: POLL_TIME_DATA");
					}
					else if(gps_task_event_msgq_item.ev == TIME_UPDATE_COMPLETE)	//RTC updated, wait for timepulse
					{
						//success
						time_task_add_event_with_timestamp(GPS_TIME_SYNC, gps_task_event_msgq_item.timestamp_unix, gps_task_event_msgq_item.timestamp_msec, gps_task_event_msgq_item.k_cycle);
						state = POST_CORRECTION;
						LOG_INF("Change state: POST_CORRECTION");
						break;

					}
						
				}
				else
				{
					LOG_INF("No timepulse arrived for 5 minutes, change state to GNSS wakeup");
					state = IDLE;
					LOG_INF("Change state: IDLE");
					gps_task_add_event(GPS_WAKEUP_REQUEST);

				}
				
				break;
				
			case POST_CORRECTION:
				//Store last active time for selecting hot/warm/cold start at next update
				time_local_get_date_and_time_t(&time_t_last_wake_time);
				//Shut down GPS and go to idle
				gps_disable_pps();
				//gps_send_gnss_shutdown();
				gps_enter_backup_mode(); 
				state = IDLE;
				LOG_INF("Change state: IDLE");
				break;
			default:
				LOG_ERR("Invalid state, resetting");
				state = IDLE;
				LOG_INF("Change state: IDLE");
				break;
		}
		
	}
	

}