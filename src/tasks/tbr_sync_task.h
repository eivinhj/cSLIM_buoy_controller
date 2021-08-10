#ifndef TBR_SYNC_TASK_H
#define  TBR_SYNC_TASK_H

#include <stdint.h>
#define TBR_SYNC_TASK_FIFO_MESSAGE_MAX_LENGTH 10

/**
 * @brief TBR Sync task
 * @details Responsible for synchronising the TBR time with nRF local time
 *
 * @retval 
 */
void tbr_sync_task(void);

/**
 * @brief Set TBR sync offset compared with nRF local time
 * @details The TBR is synchronised with last byte of synch message. As this 
 * transmission might not occour on precisely every 10.000000 second, the offset 
 * is stored locally and is added to TBR detections on arrival. Assuming TBR time 
 * always behind local time
 * 
 * //TODO verify that this works as expected
 *
 */
void tbr_sync_set_offset(int8_t offset_sec, uint32_t offset_usec);

/**
 * @brief Get TBR sync offset compared with nRF local time
 * @details The TBR is synchronised with last byte of synch message. As this 
 * transmission might not occour on precisely every 10.000000 second, the offset 
 * is stored locally and is added to TBR detections on arrival. Assuming TBR time 
 * always behind local time
 * 
 * //TODO verify that this works as expected
 *
 */
void tbr_sync_get_offset(int8_t* offset_sec, uint32_t* offset_usec);
#endif // TBR_SYNC_TASK_H