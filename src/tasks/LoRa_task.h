#ifndef LORA_TASK_H
#define  LORA_TASK_H

#include <stdint.h>
#include "../messages/tbr_message.h"
#include "../messages/cslim_messages.h"
#define LORA_TASK_FIFO_MESSAGE_MAX_LENGTH 100

typedef struct loRa_task_msgq_data_item {
    char message[LORA_TASK_FIFO_MESSAGE_MAX_LENGTH];
	uint8_t message_length;
} gps_task_msgq_data_item_t;

/**
 * @brief LoRa communication task
 * @details 
 *
 * @retval 
 */
void loRa_task(void);

/**
 * @brief Add tag detection to LoRa task Message queue
 * @details 
 *
 * @retval 
 */
void lora_task_add_tag_detection(tbr_tag_detection_t* tag_msg);

/**
 * @brief Add tag detection to LoRa task message queue
 * @details 
 *
 * @retval 
 */
void lora_task_add_tbr_log_message(tbr_log_t* log_msg);

/**
 * @brief Add TBR log to LoRa task message queue
 * @details 
 *
 * @retval 
 */
void lora_task_add_cslim_status_message(cslim_status_t* msg);

#endif // LORA_TASK_H