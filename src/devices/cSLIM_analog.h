/*
 * analog.h
 *
 *  Created on: 20. sep. 2018
 *      Author: mvols
 * 	Edited 2021:
 * 		Author: Eivinhj
 */

#ifndef DEVICES_HEADER_ANALOG_H_
#define DEVICES_HEADER_ANALOG_H_

#define BAD_ANALOG_READ -123
#define BATTERY_RES1_K	240
#define BATTERY_RES2_K	100

#define TGRAD			-1920

typedef enum {
	BATTERY = 0,
	TEMPERATURE
} analog_type_t;

/**
 * @brief Initialize ADC
 * @details 
 *
 * @param[in] channel ADC channel to initialize
 * 
 * @retval ADC device
 *  */
const struct device* cSLIM_adc_init( int channel );

/**
 * @brief Read analog channel
 * @details 
 *
 * @param[in] type Type of device to read (BATTERY or TEMPERATURE)
 * 
 * @retval analog value read 
 */
double analog_read(analog_type_t type);

#endif /* DEVICES_HEADER_ANALOG_H_ */
