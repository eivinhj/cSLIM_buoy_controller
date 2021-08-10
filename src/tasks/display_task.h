#ifndef DISPLAY_TASK_H
#define  DISPLAY_TASK_H

#include <stdint.h>
#define DISPLAY_TASK_FIFO_MESSAGE_MAX_LENGTH 10

typedef struct display_task_fifo_data_item {
    void *fifo_reserved;   /* 1st word reserved for use by fifo */
    char message[DISPLAY_TASK_FIFO_MESSAGE_MAX_LENGTH];
	uint8_t message_length;
} display_task_fifo_data_item_t;

/**
 * @brief Display task
 * @details updates display and toggle com to avoid display freezing up
 *
 * @retval 
 */
void display_task(void);

/**
 * @brief Request update of display
 * @details 
 *
 * @retval 
 */
void request_display_update();

#endif // DISPLAY_TASK_H