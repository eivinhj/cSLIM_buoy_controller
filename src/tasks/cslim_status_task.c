#include "cslim_status_task.h"


#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "time_task.h"
#include "../time/time_local.h"
#include "mqtt_task.h"
#include "LoRa_task.h"
#include "../messages/cslim_messages.h"
#include "../devices/tbr.h"
#include "../devices/cSLIM_analog.h"
#include "../devices/rv-3032-c7.h"

LOG_MODULE_REGISTER(cslim_status_task, LOG_LEVEL_DBG);

K_MSGQ_DEFINE(nav_data_msgq, sizeof(nav_data_t), 10, 64);

static uint16_t tbr_serial_number = 0;

int16_t previous_temperature_corrected = 0;

void cslim_status_task_add_nav_message(nav_data_t* nav_data)
{
	while (k_msgq_put(&nav_data_msgq, nav_data, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&nav_data_msgq);
        }

}

void cslim_status_set_tbr_serial_number(uint16_t serial_number)
{
	tbr_serial_number = serial_number;
}

void cslim_status_task()
{
	LOG_INF("Is thread ID %x", (int) k_current_get());
	cslim_status_t cslim_status; 
	nav_data_t 		nav_data;
	cslim_status.air_temperature = 0;
	cslim_status.battery_status = 0; //todo read battery

	int status = k_msgq_get(&nav_data_msgq, &nav_data, K_FOREVER);

	while(1)
	{
		//todo wait for nav msg, make cslim status message and send to mqtt
		LOG_INF("Adding cSLIM status msg");
		time_t local_time; 
		time_local_get_date_and_time_t(&local_time);
		cslim_status.air_temperature				= rv3032_get_temp();
		cslim_status.battery_status					= (int) (analog_read(BATTERY) * 30); //TODO Adapt this to give battery % 0-100 more accurate
		cslim_status.timestamp_unix					= local_time;
		cslim_status.tbr_serial_number				= get_tbr_serial_number();

		if (status == 0)
		{

			cslim_status.number_of_tracked_satelittes	= nav_data.numSV;
			cslim_status.fix							= nav_data.fix;
			cslim_status.pdop							= nav_data.pDOP;
			cslim_status.latitude						= nav_data.latitude;
			cslim_status.longitude						= nav_data.longitude;			
		}

		//Send message
		mqtt_task_add_cslim_status_message(&cslim_status);
		lora_task_add_cslim_status_message(&cslim_status);

		if(abs(previous_temperature_corrected - cslim_status.air_temperature) > TEMPERATURE_TRESHOLD)
		{
			previous_temperature_corrected = cslim_status.air_temperature;
			time_task_add_event(RTC_TEMPERATURE_CHANGED);
			LOG_INF("Temperature changed, update time");
		}

		status = k_msgq_get(&nav_data_msgq, &nav_data, K_MINUTES(10));
	}
}