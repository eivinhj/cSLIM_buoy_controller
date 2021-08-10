#ifndef TIME_TASK_H
#define  TIME_TASK_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#define TIME_TASK_FIFO_MESSAGE_MAX_LENGTH 10

typedef enum time_task_state_e {
    IDLE2,
    GPS_WAKEUP,
    UPDATE_TIME_DATA_GPS,
    WAIT_FOR_GPS_SYNC,
    ON_RTC_TIME, 
    ON_LOCAL_TIME
} time_task_state;

typedef enum time_task_event_e {
    GPS_TIME_SYNC,
    GPS_UPDATE_REJECTED, 
    GPS_UPDATE_ACCEPTED, 
    GPS_TIMEPULSE,
    RTC_TIMEPULSE,
    RTC_TEMPERATURE_CHANGED
} time_task_event;

typedef struct time_task_msgq_data_item {
    time_task_event      ev;
    time_t              timestamp_unix;
    uint32_t            timestamp_msec;
    uint32_t            k_cycle;
} time_task_event_msgq_data_item_t;

typedef struct time_task_time_msgq_data_item {
    struct tm           tm_task_time;               //For date and time down to seconds (rest will be handled by timepulse signal)
    time_t              timestamp_unix;
    uint32_t            timestamp_msec;
    uint32_t            k_cycle;
} time_task_time_msgq_data_item_t;

/**
 * @brief Add time update message to time_task
 * @details Includes time struct (struct tm), OS cycles and OS ticks
 *
 * @param[in] time_data  Message queue struct containing time data described above
 * 
 */
void time_task_add_time_msg(time_task_time_msgq_data_item_t* time_data);

/**
 * @brief Add external event of RTC task
 * @details Used to send events to RTC task, needed to syncronize with GPS task
 *
 * @param[in] ev Task event type
 * 
 */
void time_task_add_event(time_task_event ev);

/**
 * @brief Add external event of RTC task with precise time
 * @details Used to send events to RTC task, needed to syncronize with GPS task
 * Includes OS cycles and ticks to sync time with GPS
 *
 * @param[in] ev Task event type
 * 
 * @param[in] cycle OS cycles since startup
 * 
 * @param[in] tick OS kernel ticks
 * 
 */
void time_task_add_event_with_timestamp(time_task_event ev, time_t ts_unix, uint32_t ts_msec, uint32_t k_cycle);




/**
 * @brief RTC task
 * @details RTC OS task to syncronize local, rtc and GPS time to get precise local time at events.
 * 
 * 
 * 
 */
void time_task(void);

#endif // TIME_TASK_H