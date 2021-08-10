/*
 * rv-3032-c7.h
 *
 * Created: 10.05.21
 *  Author: eivinhj
 */ 
#ifndef RV_3032_C7_H
#define RV_3032_C7_H

#include <stdint.h>
#include <time.h>
#include <drivers/gpio.h>

#include "rv-3032-c7_defines.h"



/**
 * @brief Initialize RV3032 
 * @details Initialize I2C, get device bindings for RV3032 GPIO (timepulse and nINT)
 * Interrupt handlers has to be set by calling rv3032_register_callbacks()
 *
 * @retval	
 */
int rv3032_init(void);

/**
 * @brief Register RV3032 callbacks
 * @details Register callbacks for timepulse and nINT interrupts
 *
 * @param[in] rtc_clkout_callback Callback handler for Clockout interrupt
 * 
 * @param[in] rtc_nint_callback Callback handler for nINT interrupt
 * 
 * @retval  0 if successful, negative errno code on failure.
 * 
 */
int rv3032_register_callbacks(gpio_callback_handler_t rtc_clkout_callback, gpio_callback_handler_t rtc_nint_callback);

/**
 * @brief Set RTC time
 * @details Set hour, minutes and seconds in RTC. 
 * Setting seconds register reset RTC internal counters to zero which reset timepulse syncronization
 *
 * @param[in] time Struct containing time info
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_set_time(struct tm* time);

/**
 * @brief Set RTC date
 * @details Set year, minute and seconds in RTC
 *
 * @param[in] time	Struct containing date info. Mark: year is since 1970, month start at 0
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_set_date(struct tm* time);

/**
 * @brief Set RTC date and time
 * @details Set year, month, day hour, minutes and seconds in RTC. 
 * Setting seconds register reset RTC internal counters to zero which reset timepulse syncronization
 *
 * @param[in] time	Struct containing time and date info. Mark: year is since 1970, month start at 0
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_set_date_and_time(struct tm* time);

/**
 * @brief  Get RTC date and time
 * @details Get year, month, day hour, minutes and seconds in RTC. 
 *
 * @param[in]  time	Struct for storing time and date info. Mark: year is since 1970, month start at 0
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
struct tm rv3032_get_date_and_time();

/**
 * @brief Set RTC seconds
 * @details Setting seconds register reset RTC internal counters to zero which reset timepulse syncronization
 *
 * @param[in] seconds Seconds [0..59]
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_set_seconds(uint8_t seconds);

/**
 * @brief Set RTC clockout frequency (low frequency)
 * @details 
 *
 * @param[in] frequency One of available frequencies in RV3032_CLKOUT_FREQUENCY
 * 
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_set_clockout_frequency_lf(RV3032_CLKOUT_FREQUENCY frequency);

/**
 * @brief Enable RTC CLKOUT
 * @details 
 *
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_clkout_enable();

/**
 * @brief Disable RTC clockout
 * @details 
 *
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_clkout_disable();

/**
 * @brief Get RTC temperature
 * @details 
 * More precise temperature can be measured, however it is not implemented. 
 *
 * @retval temperature in degree celcius
 */
int rv3032_get_temp();

/**
 * @brief Enable RTC clock reset on EVI 
 * @details  Sets up the RTC to reset internal counters on rising edge on EVI pin.
 *  Use to calibrate RTC with more precise time pulse
 *
 * @retval 0 if successfull, -EIO General input / output error.
 */
int rv3032_enable_evi_clock_reset();

/**
 * @brief Correct RTC aging
 * @details Use measured frequency of RTC to calculate and add 
 *
 * @retval 0 if successfull, -EIO General input / output error, -1 if age correction is to large.
 */
int rv3032_correct_aging(int32_t rtc_frequency_uhz);



#endif //RV_3032_C7_H