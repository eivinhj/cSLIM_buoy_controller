/*
 * time_conversions.c
 *
 *  Created on: Feb 2020
 *      Author: MariusSR
 */

#include "time_conversions.h"


unsigned long time_manager_unixTimestamp(int year, int month, int day, int hour, int min, int sec) {
    const short days_since_beginning_of_year[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

    int leap_years = ((year-1)-1968)/4
                     - ((year-1)-1900)/100
                     + ((year-1)-1600)/400;

    long days_since_1970 = (year-1970)*365 + leap_years
                           + days_since_beginning_of_year[month-1] + day-1;

    if ( (month>2) && (year%4==0 && (year%100!=0 || year%400==0)) )
        days_since_1970 += 1; /* +leap day, if year is a leap year */

    return sec + 60 * ( min + 60 * (hour + 24*days_since_1970) );
}


bool time_manager_get_utc_time_from_gps_time(uint16_t gps_week, uint32_t gps_tow, uint16_t* utc_year, uint8_t* utc_month, uint8_t* utc_day, uint8_t* utc_hour, uint8_t* utc_minute, uint8_t* utc_seconds) {
    double julian_date = 0;
    unsigned char utc_offset = 0;
    bool result;

    if (gps_tow < 0 || gps_tow > 604800.0) return false;

    result = time_manager_get_julian_date_from_gps_time(gps_week, gps_tow, utc_offset, &julian_date);
    if (!result) return false;

    result = time_manager_get_utc_time_from_julian_date(julian_date, utc_year, utc_month, utc_day, utc_hour, utc_minute, utc_seconds);
        if (!result) return false;

    return true;
}


bool time_manager_get_julian_date_from_gps_time(const uint16_t gps_week, const uint32_t gps_tow, const uint8_t utc_offset, double* julian_date) {
    if (gps_tow < 0 || gps_tow > 604800) return false;

    // GPS time is ahead of UTC time and Julian time by the UTC offset
    *julian_date = ((gps_week + (gps_tow-utc_offset)/604800.0)*7.0 + TIME_MANAGER_JULIAN_DATE_START_OF_GPS_TIME);
    return true;
}

bool time_manager_get_utc_time_from_julian_date(const double julian_date, uint16_t* utc_year, uint8_t* utc_month, uint8_t* utc_day, uint8_t* utc_hour, uint8_t* utc_minute, uint8_t* utc_seconds) {
    int a, b, c, d, e;

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t days_in_month = 0;
    double td;
    double seconds;
    bool result;

    if ( julian_date < 0 ) return false;

    a = (int)( julian_date + 0.5);
    b = a + 1537;
    c = (int)( ((double)b - 122.1)/365.25 );
    d = (int)( 365.25*c );
    e = (int)( ((double)(b-d))/30.6001 );

//    td      = b -d - (int)(30.6001*e) + fmod( (julian_date+0.5), 1.0);    // days
    double julian_date_rem = (julian_date+0.5) - (int)(julian_date+0.5);
    td      = b - d - (int)(30.6001*e) + julian_date_rem;    // days
    day     = (uint8_t)td;
    td     -= day;
    td     *= 24.0;  // hours
    hour    = (uint8_t)td;
    td     -= hour;
    td     *= 60.0;  // minutes
    minute	= (uint8_t) td;
    td	   -= minute;
    td	   *= 60.0;
    seconds = td;
    month   = (uint8_t)(  e - 1 - 12*(int)(e/14));
    year    = (uint16_t)( c - 4715 - (int)( (7.0+(double)month) / 10.0));

    // Control for rollover issues
    if ( seconds >= 60.0) {
        seconds -= 60;
        ++minute;
        if ( minute >= 60) {
            minute -= 60;
            ++hour;
            if ( hour >= 24) {
                hour -= 24;
                ++day;

                result = time_manager_get_number_of_days_in_month( year, month, &days_in_month );
                if ( !result ) return false;

                if ( day > days_in_month ) {
                    day = 0;
                    ++month;
                    if ( month > 12 ) {
                        month = 1;
                        ++year;
                    }
                }
            }
        }
    }

    *utc_year    = year;
    *utc_month   = month;
    *utc_day     = day;
    *utc_hour    = hour;
    *utc_minute  = minute;
    *utc_seconds = (float)seconds;

    return true;
}


bool time_manager_get_number_of_days_in_month( const uint16_t year, const uint8_t month, uint8_t* days_in_month) {
    bool is_leap_year;
    uint8_t utmp = 0;

    is_leap_year = time_manager_is_leap_year( year );

    switch( month ) {
        case 1: utmp  = TIME_MANAGER_DAYS_IN_JAN; break;
        case 2: utmp  = is_leap_year ?  29 : 28  ; break;
        case 3: utmp  = TIME_MANAGER_DAYS_IN_MAR; break;
        case 4: utmp  = TIME_MANAGER_DAYS_IN_APR; break;
        case 5: utmp  = TIME_MANAGER_DAYS_IN_MAY; break;
        case 6: utmp  = TIME_MANAGER_DAYS_IN_JUN; break;
        case 7: utmp  = TIME_MANAGER_DAYS_IN_JUL; break;
        case 8: utmp  = TIME_MANAGER_DAYS_IN_AUG; break;
        case 9: utmp  = TIME_MANAGER_DAYS_IN_SEP; break;
        case 10: utmp = TIME_MANAGER_DAYS_IN_OCT; break;
        case 11: utmp = TIME_MANAGER_DAYS_IN_NOV; break;
        case 12: utmp = TIME_MANAGER_DAYS_IN_DEC; break;
        default: return false;
    }

    *days_in_month = utmp;
    return true;
}


bool time_manager_is_leap_year ( const uint16_t year ) {
    bool is_leap_year = false;
    if ( (year % 4) == 0 ) {
        is_leap_year = true;
        if ( (year % 100) == 0 ) {
            is_leap_year = ( (year % 400) == 0 );
        }
    }
    return is_leap_year;
}
