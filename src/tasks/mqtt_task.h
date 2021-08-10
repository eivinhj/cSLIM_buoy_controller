#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#define MQTT_TASK_RECEIVE_DATA_MAX_LENGTH 1024

#include <stdint.h>
#include "../messages/tbr_message.h"
#include "../messages/cslim_messages.h"

typedef struct mqtt_task_receive_data_item {
	void *fifo_reserved;
    char message[MQTT_TASK_RECEIVE_DATA_MAX_LENGTH];
	uint8_t message_length;
} mqtt_task_receive_data_item_t;

/**
 * @brief Add tag detection to MQTT task
 * @details 
 *
 * @retval 
 */
void mqtt_task_add_tag_detection(tbr_tag_detection_t* tag_msg);

/**
 * @brief Add TBR log to MQTT task
 * @details 
 *
 * @retval 
 */
void mqtt_task_add_tbr_log_message(tbr_log_t* log_msg);

/**
 * @brief Add cSLIM status message to MQTT task
 * @details 
 *
 * @retval 
 */
void mqtt_task_add_cslim_status_message(cslim_status_t* msg);

/**
 * @brief MQTT task
 * @details 
 *
 * @retval 
 */
void mqtt_task();
#endif //MQTT_TASK_H