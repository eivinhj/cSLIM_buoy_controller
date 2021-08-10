#include <time.h>
#include <stdint.h>

void time_local_init();

int time_local_add_rtc_tick(uint32_t* k_cycle);

int time_local_set_date_and_time(time_t* lt, uint32_t* k_cycle);

int time_local_set_date_and_time_at_gps_correction(time_t* gps_time);
/**
 * @brief Get local date and time time_t
 * @details Get local date and time down to seconds
 *
 * @param[in] lt time_t to store time data down to seconds
 * 
* @retval -1 if timedata is not correct
 * @retval 0 if timedata is set
 */
int time_local_get_date_and_time_t(time_t* lt);
/**
 * @brief Get local date and time struct tm
 * @details Get local date and time down to seconds
 *
 * @param[in] lt struct tm to store time data down to seconds
 * 
* @retval -1 if timedata is not correct
 * @retval 0 if timedata is set
 */
int time_local_get_date_and_time_tm(struct tm* lt);
/**
 * @brief Get local time with microsecond precision
 * @details Used to get time of events registered with cycles and ticks
 * Microseconds might not be accurate, but should never be more than 500us off
 *
 * @param[in] lt time_t to store time data down to seconds
 * 
 * @param[in] us used to store microseconds of time value
 * 
 * @retval -1 if timedata is not correct
 * @retval 0 if timedata is set
 * 
 */

int get_precise_local_date_and_time_t(time_t* lt, uint32_t* us, uint32_t* k_cycle_at_event);

/**
 * @brief Get local time with microsecond precision
 * @details Used to get time of events registered with cycles and ticks
 * Microseconds might not be accurate, but should never be more than 500us off
 *
 * @param[in] lt struct tm to store time data down to seconds
 * 
 * @param[in] us used to store microseconds of time value
 * 
 * @retval -1 if timedata is not correct
 * @retval 0 if timedata is set
 * 
 */
int get_precise_local_date_and_time_tm(struct tm* lt, uint32_t* us, uint32_t* k_cycle_at_event);

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
int time_local_set_rtc_estimated_frequency(long double frequency);

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
long double time_local_get_rtc_estimated_frequency();

