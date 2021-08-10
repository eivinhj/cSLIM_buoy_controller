/*
 * time_manager.c
 *
 *  Created on: Mar 2020
 *      Author: MariusSR
 */

//#include "../resource managers/app_manager.h"  //TODO
#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include "time_manager.h"
#include "../devices/ublox_gps.h"
#include "../devices/cSLIM_analog.h"
#include "../drivers/cSLIM_rs232.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(time_manager);


//#ifdef USE_RADIO


time_data_t	 		    running_tstamp = {.valid = false, .unix_tstamp = 0};
time_data_t	 			ref_tstamp = {.valid = false};
time_data_t             gnss_correction_tstamp = {.valid = false};
//#endif

/*
 * Private function declaration
 */


/*
 * Private functions
 */

void time_manager_update_utc_and_unix_time(time_data_t* tim_data)
{
    tim_data->valid = time_manager_get_utc_time_from_gps_time(tim_data->week, tim_data->tow_ms/1000, &(tim_data->year),
                                                                    &(tim_data->month), &(tim_data->day), &(tim_data->hour), &(tim_data->min), &(tim_data->sec));

    tim_data->unix_tstamp = time_manager_unixTimestamp(tim_data->year, tim_data->month, tim_data->day,
                                                             tim_data->hour, tim_data->min, tim_data->sec);
}


/*
 * Public navigation functions
 */


uint32_t time_manager_getLocalUnixTime( void ) {
	return running_tstamp.unix_tstamp;
}
