/*
 * time_conversions.h
 *
 *  Created on: Feb 2020
 *      Author: MariusSR
 */

#ifndef SSM_V4_LP_TIME_CONVERSION_H
#define SSM_V4_LP_TIME_CONVERSION_H

#include <stdint.h>
#include <stdbool.h>

#define UTC_OFFSET 1

#define TIME_MANAGER_JULIAN_DATE_START_OF_GPS_TIME (2444244.5)  // [days]
#define TIME_MANAGER_JULIAN_DATE_START_OF_PC_TIME  (2440587.5)  // [days]

#define TIME_MANAGER_DAYS_IN_JAN 31
#define TIME_MANAGER_DAYS_IN_MAR 31
#define TIME_MANAGER_DAYS_IN_APR 30
#define TIME_MANAGER_DAYS_IN_MAY 31
#define TIME_MANAGER_DAYS_IN_JUN 30
#define TIME_MANAGER_DAYS_IN_JUL 31
#define TIME_MANAGER_DAYS_IN_AUG 31
#define TIME_MANAGER_DAYS_IN_SEP 30
#define TIME_MANAGER_DAYS_IN_OCT 31
#define TIME_MANAGER_DAYS_IN_NOV 30
#define TIME_MANAGER_DAYS_IN_DEC 31


//TODO Comment this
/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
unsigned long time_manager_unixTimestamp(int year, int month, int day, int hour, int min, int sec);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_unix_time_from_gps_time(uint16_t gps_week, uint32_t gps_tow, uint32_t* unix_time);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_get_utc_time_from_gps_time( uint16_t gps_week, uint32_t gps_tow, uint16_t* utc_year, uint8_t* utc_month, uint8_t* utc_day, uint8_t* utc_hour, uint8_t* utc_minute, uint8_t* utc_seconds);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_get_julian_date_from_gps_time( const uint16_t gps_week, const uint32_t gps_tow, const uint8_t utc_offset, double* julian_date);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_determine_utc_offset( double julian_date, uint8_t* utc_offset);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_get_utc_time_from_julian_date( const double julian_date, uint16_t* utc_year, uint8_t* utc_month, uint8_t* utc_day, uint8_t* utc_hour, uint8_t* utc_minute, uint8_t* utc_seconds);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_get_number_of_days_in_month( const uint16_t year, const uint8_t month, uint8_t* days_in_month);

/**
 * @brief 
 * @details 
 *
 * @param[in] void		void
 */
bool          time_manager_is_leap_year ( const uint16_t year );

#endif /* SSM_V4_LP_TIME_MANAGER_H */
