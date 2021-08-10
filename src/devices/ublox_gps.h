/*
 * ublox_gps.h
 *
 *  Created on: Apr 25, 2017
 *      Author: Waseemh
 *  Updated by Rundhovde 2020
 *  Updated by eivinhj 2021 for use with cSLIM
 */

#ifndef SRC_UBLOX_GPS_H_
#define SRC_UBLOX_GPS_H_

#include "../drivers/cSLIM_SPI.h"

#include "ublox_msg.h"
#include "display.h"

#include <stdbool.h>
#include <stdint.h>
#include <drivers/gpio.h>



/*
 * Macros
 */
#define 	RETRY	         25
#define     HEADER_LENGTH    5  		// Used to offset the UBX messages to the payload offset. is in reality 6

#define     USE_POWER_SAVING 	true
#define     USE_FULL_POWER 	    false   //only used if USE_POWER_SAVING is false

#define 	USE_PMS			 	true	// Use the newer CFG-PMS instead of CFG-PM2
//#define     USE_PMS_BALANCED	true    // Må ha USE_PMS enabled

//


#define     USE_TIME_PULSE_SIGNAL true

#define     CLEAR_SETTINGS_ON_BOOT true

//#define PUBLISH_NAV_PVT_MSG true
//#define PUBLISH_TIM_TP_MSG  true
//#define PUBLISH_TIM_TM2_MSG true

#define RISING_EDGE_TRIGGERED_INT true      // Used to determine which edge of the TM2 data to use for time correction

//#define USE_WAKEUP_MSG      true

#ifdef DEBUG_GNSS_test
#define DEBUG_GNSS_FLAG 1
#else
#define DEBUG_GNSS_FLAG 0
#endif

/*
 * Structs and typedefs
 */

typedef struct {
    bool        valid;
    uint32_t    unix_tstamp;
    int32_t     nano;
    uint32_t    tAcc;
} unix_time_t;

typedef struct {
    uint8_t     header[2];
    uint8_t     class;
    uint8_t     id;
    uint8_t     length;
    uint8_t     payload[128];    
    uint8_t     checksum[2];

    uint32_t    recv_time_local;    // used to track the time instant the msg was received
    uint32_t    BURTC_subsec_tick;  // used to track the time instant the msg was received in sub sec
} ubx_msg_t;

typedef struct {
	bool		valid;
	bool        valid_date;
	bool        valid_time;

	bool        confirmed_date;     // not to be used without testing, used for debugging at the moment
	bool        confirmed_time;     // not to be used without testing, used for debugging at the moment
	bool        fully_resolved;     // not to be used without testing, used for debugging at the moment
	bool        gnss_fix_ok;        // not to be used without testing, used for debugging at the moment

	uint32_t	unix_tstamp;
	uint16_t	year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t		min;
	uint8_t		sec;
	uint8_t		t_flags;	//added later on
	uint32_t	tAcc;		//added later on
	int32_t 	nano;		//added later on
	uint8_t		numSV;		//added later on
	uint16_t	pDOP;		//added later later on
	uint32_t	longitude;
	uint32_t	latitude;
	uint32_t	height;
	uint8_t		fix;		//added later later later on

    uint32_t    recv_time_local;    // used to track the time instant the msg was received
    uint32_t    BURTC_subsec_tick;  // used to track the time instant the msg was received in sub sec
} nav_data_t;


typedef struct {
    bool        valid;
    uint8_t     cno_min;
    uint8_t     cno_max;
    uint8_t     numSV;
    uint8_t     numSV_over_40_dBHz;
} sat_data_t;


typedef struct {
    bool        valid;
    uint32_t    unix_tstamp;
    uint32_t    tow_ms;
    uint32_t    tow_sub_ms;
    uint32_t    quantisation_error;
    uint16_t    week;
    uint8_t     flags;
    uint8_t     ref_info;
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     min;
    uint8_t     sec;
} time_data_t;

typedef struct {
    bool        valid;
    uint8_t     channel;
    uint8_t     flags;
    uint16_t    rising_edge_count;
    uint16_t    week_rising_edge;
    uint16_t    week_falling_edge;
    uint32_t    tow_ms_rising_edge;
    uint32_t    tow_ms_falling_edge;
    uint32_t    tow_sub_ms_rising_edge;
    uint32_t    tow_sub_ms_falling_edge;
    uint32_t    accuracy_estimate;

    uint32_t    unix_tstamp;
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     min;
    uint8_t     sec;
} time_tp2_data_t;

typedef struct {
    bool        valid;
    bool        is_accepted;
    uint8_t     class;
    uint8_t     id;
} ack_data_t;


/*
 * public functions
 */

//TODO Check if all functions are used, maybe remove if not (if future use is unlikely)
//TODO Add comments to functions


bool            gps_init(gpio_callback_handler_t gpio_timepulse_cb);
bool            gps_get_firmware_version();
bool            gps_get_nav5( void );

void 			gps_poll_nav_data ( void );
void            gps_poll_sat_status ( void );
sat_data_t      gps_get_sat_data ( void );
//nav_data_t 		gps_get_nav_data( void );

bool            gps_print_PMS_config( void );
bool            gps_print_PRT_config( void );

void            gps_int_pin_set( void );
void            gps_int_pin_clear( void );
void 			gps_int_pin_toggle( void );

bool            gps_read_spi_data (ubx_msg_t*   recv_msg); 

nav_data_t      get_latest_nav_data( void );
time_data_t     get_latest_tim_data( void );
bool            is_new_nav_data_available( void );
bool            is_new_nav_data_available_no_fix_or_acc_requirement( void );
bool            is_new_sat_data_available( void );
bool            is_new_tim_data_available( void );
bool            is_new_accurate_tim_data_available( void );
void            gps_int_read_potential_queue( void );
void            gps_read_all_spi_data ( void );
void            gps_poll_tim_status ( void );
bool            gps_poll_tim_status_polling_based (ubx_msg_t*   recv_msg);
void            update_ack_data(const ubx_msg_t* msg, ack_data_t* ack_data, const bool is_accepted);

bool            gps_disable_pps( void );
bool            gps_enable_pps( void );

void            gps_enter_backup_mode( void );

void            gps_send_gnss_shutdown( void );
void            gps_send_gnss_wakeup_hot_start( void );
void            gps_send_gnss_wakeup_warm_start( void );
void            gps_send_gnss_wakeup_cold_start( void );

bool            gps_change_pms_wakeup_interval( uint16_t wakeup_interval_in_sec );
//bool            gps_poll_nav_data_polling_based (void);

void            gps_clear_tim_data( void );


void            update_time_data_unix(nav_data_t* nav_data, unix_time_t* time_data);
void            parse_tim_data(ubx_msg_t* msg, time_data_t* tim_data);
void            update_nav_data(ubx_msg_t* msg, nav_data_t* nav_data);
void            update_sat_data(const ubx_msg_t* msg, sat_data_t* sat_data);

void            gps_calculate_checksum(uint8_t* message, uint8_t message_length, uint8_t* ck_a, uint8_t* ck_b); 

#endif /* SRC_UBLOX_GPS_H_ */
