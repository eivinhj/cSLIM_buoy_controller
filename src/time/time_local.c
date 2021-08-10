#include "time_local.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/counter.h>
#include "../devices/rv-3032-c7.h"


LOG_MODULE_REGISTER(time_local, LOG_LEVEL_INF);

///////////////////////////////////////
//
///////////////////////////////////////

K_TIMER_DEFINE(local_time_counter, NULL, NULL);

////////////
//Local variables
///////////
time_t time_t_local_time;
time_t time_at_gps_correction;
int time_t_local_time_valid = -1;

long double rtc_estimated_frequency = 1.0; //compared with GPS to find accurate

uint32_t k_cycle_at_last_rtc_tick;
uint32_t k_tick_at_last_rtc_tick;

struct k_mutex time_local_mutex;

void time_local_init()
{
	//k_timer_start(&local_time_counter, K_MSEC(200), K_NO_WAIT);
	k_mutex_init(&time_local_mutex);

}

int time_local_add_rtc_tick(uint32_t* k_cycle)
{
	LOG_DBG("add tick, k_cycle: %d", *k_cycle);
	while(k_mutex_lock(&time_local_mutex, K_MSEC(500)))
	{
		LOG_ERR("Could not get time_local mutex");
	}
	//LOG_INF("RTC Clockout Callback at %" PRIu32 "\n", k_cycle_get_32());

	uint32_t usec_since_last_timepulse = k_cyc_to_us_near32(*k_cycle - k_cycle_at_last_rtc_tick) ;
	if(usec_since_last_timepulse < 700000) //double timepulse? This might happen on GPS update
	{
		LOG_ERR("Double timepulse from RTC/GPS? usec is %d", usec_since_last_timepulse);
		return 1;
	}

	if(usec_since_last_timepulse > 1100000) //missed timepulse? This might happen on GPS update
	{
		LOG_ERR("Missing timepulse from RTC? usec is %d", usec_since_last_timepulse);
		time_t_local_time++;
	}

	time_t_local_time++;
	k_cycle_at_last_rtc_tick = *k_cycle;
	k_mutex_unlock(&time_local_mutex);
	return 0;
}

uint32_t get_us_elapsed_after_rtc_tick(uint32_t* k_cycle_at_event)
{
	return (k_cyc_to_us_near32(*k_cycle_at_event - k_cycle_at_last_rtc_tick));
}


int time_local_set_date_and_time(time_t* lt, uint32_t* k_cycle)
{
	LOG_DBG("Set date and time, kcycle: %d", *k_cycle);
	time_t_local_time = *lt;
	time_t_local_time_valid = 0; 
	k_cycle_at_last_rtc_tick = *k_cycle;
	return 0;
}

int time_local_set_date_and_time_at_gps_correction(time_t* gps_time)
{
	time_at_gps_correction = *gps_time;
	return 0;
}

int time_local_get_date_and_time_t(time_t* lt)
{
	if(time_t_local_time_valid != 0)
	{
		return time_t_local_time_valid;
	}
	uint32_t k_cycle = k_cycle_get_32();
	uint32_t us;
	return get_precise_local_date_and_time_t(lt, &us, &k_cycle);

}

int time_local_get_date_and_time_tm(struct tm* lt)
{
	if(time_t_local_time_valid != 0)
	{
		return time_t_local_time_valid;
	}
	time_t local_time;
	uint32_t us;
	uint32_t k_cycle = k_cycle_get_32();
	get_precise_local_date_and_time_t(&local_time, &us, &k_cycle);
	*lt = *localtime(&local_time);
	return time_t_local_time_valid;

}

int get_precise_local_date_and_time_t(time_t* lt, uint32_t* us, uint32_t* k_cycle_at_event)
{
	if(time_t_local_time_valid != 0)
	{
		return time_t_local_time_valid;
	}
	//Take mutex
	if(k_mutex_lock(&time_local_mutex, K_USEC(250)))
	{
		LOG_ERR("Could not get time_local mutex in time");
		return -1;
	}
	*us = get_us_elapsed_after_rtc_tick(k_cycle_at_event);
	*lt = time_t_local_time;

	//Release mutex
	k_mutex_unlock(&time_local_mutex);


	uint16_t seconds_passed = *us / USEC_PER_SEC;

	*us = *us % USEC_PER_SEC;

	*lt = time_t_local_time + seconds_passed;
	

	//correct with estimated RTC clockout frequency (There is a problem with offset seconds, giving wrong estimated frequency and making this not useful)

	uint64_t lt_temp_us =(uint64_t)( (long double)(*lt - time_at_gps_correction)*USEC_PER_SEC +*us) / (rtc_estimated_frequency);
	*us = lt_temp_us % USEC_PER_SEC;
	*lt = lt_temp_us/USEC_PER_SEC + time_at_gps_correction;



	return time_t_local_time_valid;
}

int get_precise_local_date_and_time_tm(struct tm* lt, uint32_t* us, uint32_t* k_cycle_at_event)
{
	if(time_t_local_time_valid != 0)
	{
		return time_t_local_time_valid;
	}
	time_t local_time;
	get_precise_local_date_and_time_t(&local_time, us, k_cycle_at_event);
	*lt = *localtime(&local_time);
	return time_t_local_time_valid;
}

int time_local_set_rtc_estimated_frequency(long double frequency)
{
	if(frequency>1.1 || frequency < 0.9)
	{
		LOG_ERR("Frequency is out of range");
		return -1;
	}
	rtc_estimated_frequency = frequency;
	return 0;
}

long double time_local_get_rtc_estimated_frequency()
{
	return rtc_estimated_frequency;
}

