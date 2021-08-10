#include "mqtt_task.h"

#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(cSLIM_mqtt_task, LOG_LEVEL_DBG);

#include "../mqtt/cSLIM_MQTT.h"
#include "display_task.h"
#include "../devices/display.h"



K_MSGQ_DEFINE(tbr_tag_detections, sizeof(tbr_tag_detection_t), 20, 32);
K_MSGQ_DEFINE(tbr_log_messages, sizeof(tbr_log_t), 20, 32);
K_MSGQ_DEFINE(cslim_status_messages, sizeof(cslim_status_t), 10, 32);

#define MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH 200 //match with size of tbr_tag_detections and tbr_log_ above, much fit messages plus headers

/////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////
bool mqtt_init_complete = false;

/////////////////////////////////////////////////
// Local function declarations
/////////////////////////////////////////////////

int mqtt_task_send_tag_detections();
int mqtt_task_send_tbr_log();
int mqtt_task_send_cslim_status();

/////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////

void mqtt_task_add_tag_detection(tbr_tag_detection_t* tag_msg)
{
	int err = k_msgq_put(&tbr_tag_detections, tag_msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(mqtt_init_complete)
		{
			mqtt_task_send_tag_detections();
			k_msgq_put(&tbr_tag_detections, tag_msg, K_NO_WAIT);
		}
		
	}
}

void mqtt_task_add_tbr_log_message(tbr_log_t* log_msg)
{
	int err = k_msgq_put(&tbr_log_messages, log_msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(mqtt_init_complete)
		{
			mqtt_task_send_tbr_log();
			k_msgq_put(&tbr_log_messages, log_msg, K_NO_WAIT);
		}
		
	}
}

void mqtt_task_add_cslim_status_message(cslim_status_t* msg)
{
	int err = k_msgq_put(&cslim_status_messages, msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(mqtt_init_complete)
		{
			mqtt_task_send_cslim_status();
			k_msgq_put(&cslim_status_messages, msg, K_NO_WAIT);
		}
		
	}
}

void mqtt_task()
{
	LOG_INF("Is thread ID %x", (int) k_current_get());

	k_sleep(K_SECONDS(10));

	display_put_string(6, 2*12+5, "LTE: Search", font_medium);
	request_display_update();
	cSLIM_mqtt_init();
	display_put_string(6, 2*12+5, "LTE: Joined", font_medium);
	request_display_update();
	mqtt_init_complete = true;


	while(1)
	{
		cSLIM_mqtt_loop(); //return after one iteration and is called again, could be an idea to spawn two separate threads that handle the mqtt lopp and sending messages at regular intervals. 

		//Send tag detections  periodically
		mqtt_task_send_tag_detections();
		mqtt_task_send_tbr_log();
		mqtt_task_send_cslim_status();
	}
}


/////////////////////////////////////////////////
// Local functions
/////////////////////////////////////////////////

int mqtt_task_send_tag_detections()
{
	char mqtt_tag_detection_buffer[MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t mqtt_tag_detection_buffer_index = 0; 
	tbr_tag_detection_t tag_detection;

	//Check that MQTT client is ready
	if(!mqtt_init_complete)
	{
		LOG_ERR("Mqtt init not complete");
		return -1;
	} 
	int status = k_msgq_get(&tbr_tag_detections, &tag_detection, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		return status;
	}

	//TODO? Make "dummy" LoRa HEADER

	// make IOF header
	time_t reference_timestamp = tag_detection.timestamp_unix;
	//LOG_INF("TBR SERIAL: %d", tag_detection.tbr_serial_number);
	//LOG_INF("REF TS: %d", reference_timestamp);
	tbr_compressed_tag_msg_make_header(tag_detection.tbr_serial_number, reference_timestamp, mqtt_tag_detection_buffer, &mqtt_tag_detection_buffer_index);
	//add tag detections
	do {
		status = tbr_compressed_tag_msg_add_tag(&tag_detection, reference_timestamp, mqtt_tag_detection_buffer, &mqtt_tag_detection_buffer_index);
		if(status == -EINVAL || mqtt_tag_detection_buffer_index > MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH -10)
		{
			break;
		}
	} while(k_msgq_get(&tbr_tag_detections, &tag_detection, K_NO_WAIT) == 0);

	//send to mqtt 
	LOG_DBG("Send Tag detections");
	cSLIM_mqtt_data_publish(mqtt_tag_detection_buffer,(size_t) mqtt_tag_detection_buffer_index);

	if( k_msgq_peek(&tbr_tag_detections, &tag_detection) == 0) //more messages in queue
	{
		mqtt_task_send_tag_detections();
	}
	return 0; 
}

int mqtt_task_send_tbr_log()
{
	
	char mqtt_tbr_log_buffer[MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t mqtt_tbr_log_buffer_index = 0; 
	tbr_log_t tbr_log;

	//Check that MQTT client is ready
	if(!mqtt_init_complete)
	{
		LOG_ERR("Mqtt init not complete");
		return -1;
	} 

	int status = k_msgq_get(&tbr_log_messages, &tbr_log, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		//LOG_DBG("No message to send");
		return status;
	}

	//TODO Make "dummy" LoRa HEADER

	// make IOF header
	time_t reference_timestamp = tbr_log.timestamp_unix;
	tbr_compressed_log_msg_make_header(tbr_log.tbr_serial_number, reference_timestamp, mqtt_tbr_log_buffer, &mqtt_tbr_log_buffer_index);
	//add tag detections
	do {
		status = tbr_compressed_log_msg_add_tag(&tbr_log, reference_timestamp, mqtt_tbr_log_buffer, &mqtt_tbr_log_buffer_index);
		if(status == -EINVAL || mqtt_tbr_log_buffer_index > MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH -10)
		{
			break;
		}
	} while(k_msgq_get(&tbr_log_messages, &tbr_log, K_NO_WAIT) == 0);


	LOG_DBG("Send TBR Log?");
	cSLIM_mqtt_data_publish(mqtt_tbr_log_buffer,(size_t) mqtt_tbr_log_buffer_index);

	if( k_msgq_peek(&tbr_log_messages, &tbr_log) == 0) //more messages in queue
	{
		mqtt_task_send_tbr_log();
	}
	return 0; 
}

int mqtt_task_send_cslim_status()
{
	
	char mqtt_buffer[MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t mqtt_buffer_index = 0; 
	cslim_status_t msg;

	//Check that MQTT client is ready
	if(!mqtt_init_complete)
	{
		LOG_ERR("Mqtt init not complete");
		return -1;
	} 

	int status = k_msgq_get(&cslim_status_messages, &msg, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		return status;
	}

	//TODO Make "dummy" LoRa HEADER

	// make IOF header
	time_t reference_timestamp = msg.timestamp_unix;
	cslim_status_msg_make_header(msg.tbr_serial_number, reference_timestamp, mqtt_buffer, &mqtt_buffer_index);
	//add tag detections
	do {
		status = cslim_status_msg_add_msg(&msg, mqtt_buffer, &mqtt_buffer_index);
		if(status == -EINVAL || mqtt_buffer_index > MQTT_TBR_MESSAGE_BUFFER_MAX_LENGTH -10)
		{
			break;
		}
	} while(k_msgq_get(&cslim_status_messages, &msg, K_NO_WAIT) == 0);

	
	//send to mqtt 
	LOG_DBG("Send cSLIM status");
	cSLIM_mqtt_data_publish(mqtt_buffer, (size_t) mqtt_buffer_index);

	if( k_msgq_peek(&cslim_status_messages, &msg) == 0) //more messages in queue
	{
		mqtt_task_send_cslim_status();
	}
	return 0; 
}

