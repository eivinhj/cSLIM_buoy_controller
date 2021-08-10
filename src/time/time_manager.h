/*
 * time_manager.h
 *
 *  Created on: Mar 2020
 *      Author: MariusSR
 *  Updated June 2021
 *      Author: Eivinhj
 */

#ifndef SSM_V4_LP_TIME_MANAGER_H
#define SSM_V4_LP_TIME_MANAGER_H

//#include "../lmic/hal.h" //TODO

//#define USE_PVT_FOR_TIME_CORRECTION true
//#define DISABLE_PPS_BETWEEN_CORRECTIONS true

#define     CLOCK_CORRECTION_INTERVAL     120     // sec. initial interval if using dynamical intervals
#define     GNSS_POSITION_SYNC_INTERVAL   600     // sec

#define     USE_DYNAMICAL_SYNC_INTERVAL   true
#define     UPPER_TIME_ACCURACY_LIMIT               400    // us
#define     LOWER_TIME_ACCURACY_LIMIT               200    // us
//#define     UPPER_TIME_ACCURACY_LIMIT               400000    // us
//#define     LOWER_TIME_ACCURACY_LIMIT               200000    // us
#define     SMALLEST_POSSIBLE_CORRECTION_INTERVAL   20     // s
#define     LARGEST_POSSIBLE_CORRECTION_INTERVAL    900     // s

#define     USE_TEMPERATURE_TRIGGERED_INTERVAL_CHANGES true
#define     TEMPERATURE_CHANGE_LIMIT       50    // deg * 10

#define     NUMBER_OF_FIX_RETIRES           5
//#define     SEND_MISSING_FIX_MESSAGES_INTERVAL_SEC  540     //sec
#define     SEND_MISSING_FIX_MESSAGES_INTERVAL_SEC  60

#include "time_conversions.h"
#include "../devices/ublox_gps.h"

//TODO: Remove unused functions etc.
//TODO: Comment this

typedef struct {
    //gnss_manager_state_t state;
    bool     initial_time_set;
    bool     keep_gnss_awake;
    bool     gnss_time_pulse_active;
    bool     do_extraordinary_tbr_correction;
    uint16_t ticks_since_last_gnss_time_pulse;
    uint8_t  back_off_sec;

    uint16_t correction_interval;
    uint16_t ticks_since_last_correction;
    uint8_t  number_of_unanswered_tim_tp_requests;
    uint16_t ticks_since_recv_tim_msg;
    uint16_t corrections_made;
    uint16_t average_correction_interval;
    int      last_tick_error;
    int      last_time_error;
    int      largest_time_error;
    int      largest_tick_error;
    int      largest_tick_error_since_last_reset;
    int      temperature_at_last_correction;
    int      temperature_at_last_interval_correction;
    int      temperature;
    int      ppm;

    // Navigation variables
    bool     valid_position;
    bool     send_position;
    uint16_t ticks_since_last_position_update;
    uint16_t ticks_since_last_sent_position_update;
    uint8_t  number_of_fix_retries;
    uint16_t ticks_since_last_sent_missing_fix_message;

    bool     poll_tim_msg;

    bool     time_correction_in_progress;
    bool     time_tp_msg_received;
    bool     time_correction_finished;
    uint32_t  ticks_since_sent_request;
    uint32_t  ticks_since_time_tp_received;     // not in use
    long     ticks_since_started;               // todo: remove due to overflow potential

    uint16_t previous_actual_correction_interval;   // dbg: used for timing message
} time_correction_t;

//TODO Remove unneeded functions and comment the rest

// Correction functions
void                time_manager_update_utc_and_unix_time(time_data_t* tim_data);


// Get functions

uint32_t            time_manager_getLocalUnixTime( void );


#endif //SSM_V4_LP_TIME_MANAGER_H
