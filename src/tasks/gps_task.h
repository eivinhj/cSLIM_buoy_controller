#ifndef GPS_TASK_H
#define  GPS_TASK_H

#include <stdint.h>

#define GPS_TASK_FIFO_PAYLOAD_MAX_LENGTH 10

typedef enum gps_task_state_e {
    IDLE,
    WAKE_GNSS,
    UPDATE_POS,
    POLL_TIME_DATA,
    MAKE_CORRECTION,
    POST_CORRECTION
} gps_task_state;

typedef enum gps_task_event_e {
    GPS_WAKEUP_REQUEST,
    TIMEPULSE_RECEIVED,
    POSITION_UPDATE_RECEIVED,
    TIM_TP_RECEIVED, 
    READY_FOR_GPS_SYNC,
    TIME_UPDATE_REJECTED, 
    TIME_UPDATE_COMPLETE
} gps_task_event;

typedef struct gps_task_msgq_data_item {
    gps_task_event      ev;
    time_t              timestamp_unix;
    uint32_t            timestamp_msec;
    uint32_t            k_cycle;
} gps_task_event_msgq_data_item_t;

/**
 * @brief GPS Task
 * @details 
 *
 * @retval 
 */
void gps_task(void);

/**
 * @brief Add event to GPS task
 * @details 
 *
 * @retval 
 */
void gps_task_add_event(gps_task_event ev);


#endif // GPS_TASK_H