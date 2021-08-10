#ifndef TBR_DETECT_TASK_H
#define TBR_DETECT_TASK_H

#define TBR_DETECT_TASK_FIFO_MESSAGE_MAX_LENGTH 58

#include <stdint.h>

typedef struct tbr_detect_task_msgq_data_item {
    char message[TBR_DETECT_TASK_FIFO_MESSAGE_MAX_LENGTH];
	uint8_t message_length;
	int8_t tbr_offset_sec;
	uint32_t tbr_offset_usec;
} tbr_detect_task_msgq_data_item_t;

/**
 * @brief Add message to TBR detect task
 * @details 
 *
 * @retval 
 */
void tbr_detect_task_add_msg(tbr_detect_task_msgq_data_item_t* tbr_detect_msg);

/**
 * @brief TBR detect task
 * @details Responsible for receiving TBR messages containing tag detections
 * and TBR status updates. 
 *
 * @retval 
 */
void tbr_detect_task();

#endif // TBR_DETECT_TASK_H