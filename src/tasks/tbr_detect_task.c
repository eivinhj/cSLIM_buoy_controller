#include "tbr_detect_task.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>

#include "time_task.h"
#include "mqtt_task.h"
#include "LoRa_task.h"
#include "../devices/tbr.h"
#include "../messages/tbr_message.h"

LOG_MODULE_REGISTER(tbr_detect_task, LOG_LEVEL_DBG);

K_MSGQ_DEFINE(tbr_detect_task_msgq, sizeof(tbr_detect_task_msgq_data_item_t), 10, 64);

void tbr_detect_task_add_msg(tbr_detect_task_msgq_data_item_t* tbr_detect_msg)
{
	 while (k_msgq_put(&tbr_detect_task_msgq, tbr_detect_msg, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&tbr_detect_task_msgq);
        }
}

void tbr_detect_task()
{
	LOG_INF("Is thread ID %x", (int) k_current_get());

	tbr_detect_task_msgq_data_item_t tbr_detect_task_msgq_item;
	int status;
	while(1)
	{
		status = k_msgq_get(&tbr_detect_task_msgq, &tbr_detect_task_msgq_item, K_FOREVER);
		if (status == 0)
		{
			LOG_INF("Got TBR str: %s", log_strdup(tbr_detect_task_msgq_item.message));

			//make sure string starts with $
			if(tbr_detect_task_msgq_item.message[0] == '$')
			{

				//Log locally 


				//remove $ from head
				char *message = tbr_detect_task_msgq_item.message;
				message++;
				//split by , and place in message

				//get tbr_serial_number
				char* token = strtok(message, ",");
				char* temp_ptr;
				uint32_t tbr_serial_number = strtoul(token, &temp_ptr, 10);

				//get unix timestamp
				token = strtok(NULL, ",");
				uint32_t tbr_timestamp_unix = strtoul(token, &temp_ptr, 10);

				//get third part, compare to "TBR Sensor" to detect sensor log or tag detection'
				char* third_string = strtok(NULL, ",");

				if(strcmp(third_string, "TBR Sensor") == 0)
				{
					LOG_DBG("Received TBR Sensor message");
					//TBR log message
					tbr_log_t tbr_log_msg;
					tbr_log_msg.tbr_serial_number 					= tbr_serial_number;
					tbr_log_msg.timestamp_unix						= tbr_timestamp_unix + tbr_detect_task_msgq_item.tbr_offset_sec;

					//extract rest of tbr message
					tbr_log_msg.temperature							= atoi(strtok(NULL, ","));
					tbr_log_msg.average_background_noise			= atoi(strtok(NULL, ","));
					tbr_log_msg.peak_background_noise				= atoi(strtok(NULL, ","));
					tbr_log_msg.detection_SNR						= atoi(strtok(NULL, ","));

					//last part of message, remove \r
					char* last_string = strtok(NULL, ",");
					tbr_log_msg.number_of_strings_since_power_up	= atoi(strtok(last_string, "\r"));

					//form message in SLIM format, add header etc.
					//Send to MQTT task and LoRa task
					mqtt_task_add_tbr_log_message(&tbr_log_msg);
					lora_task_add_tbr_log_message(&tbr_log_msg);

				}
				else
				{
					//Tag detection
					LOG_DBG("Received Tag detection message");
					tbr_tag_detection_t tbr_tag_msg;
					tbr_tag_msg.tbr_serial_number 				= tbr_serial_number;
					tbr_tag_msg.timestamp_unix					= tbr_timestamp_unix;
					tbr_tag_msg.timestamp_ms					= atoi(third_string);
					tbr_tag_msg.transmitter_protocol_str		= strtok(NULL, ",");
					tbr_tag_msg.transmitter_id					= atoi(strtok(NULL, ","));
					tbr_tag_msg.transmitter_data_value			= atoi(strtok(NULL, ","));
					tbr_tag_msg.detection_SNR					= atoi(strtok(NULL, ","));	
					tbr_tag_msg.detection_frequency_hz			= atoi(strtok(NULL, ","));

					//last part of message, remove \r
					char* last_string = strtok(NULL, ",");
					tbr_tag_msg.number_of_strings_since_power_up = atoi(strtok(last_string, "\r"));	

					//correct for TBR offset
					tbr_tag_msg.timestamp_ms 	+=  tbr_detect_task_msgq_item.tbr_offset_usec;
					tbr_tag_msg.timestamp_unix	+= (tbr_tag_msg.timestamp_ms / USEC_PER_SEC) + tbr_detect_task_msgq_item.tbr_offset_sec;
					tbr_tag_msg.timestamp_ms 	 =  tbr_tag_msg.timestamp_ms % USEC_PER_SEC;

					//convert protocol to int
					tbr_transmitter_protocol_to_uint(&tbr_tag_msg);

					LOG_INF("Tag message is Serial: %d, Timestamp: %d.%d, Protocol: %d, tag_id: %d, Data: %d, SNR:%d, freq: %d message_num: %d", (int)tbr_tag_msg.tbr_serial_number, (int)tbr_tag_msg.timestamp_unix, (int)tbr_tag_msg.timestamp_ms, (int)tbr_tag_msg.transmitter_protocol_int, (int)tbr_tag_msg.transmitter_id, (int)tbr_tag_msg.transmitter_data_value, (int)tbr_tag_msg.detection_SNR, (int)tbr_tag_msg.detection_frequency_hz, (int)tbr_tag_msg.number_of_strings_since_power_up);
					
					mqtt_task_add_tag_detection(&tbr_tag_msg);
					lora_task_add_tag_detection(&tbr_tag_msg);
				}
			}
			

		}
		
	}
}