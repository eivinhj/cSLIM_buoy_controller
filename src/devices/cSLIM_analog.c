/*
 * analog.c
 *
 *  Created on: 20. sep. 2018
 *      Author: mvols
 * 			  : eivinhj 2021
 */

#include "cSLIM_analog.h"
#include <stdint.h>
#include <zephyr.h>
#include <drivers/adc.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(cslim_adc, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

/*
 * Zephyr devices
 */
#define ADC_DEVICE_NAME		DT_LABEL(DT_ALIAS(adcctrl))
#define ADC_RESOLUTION		10
#define ADC_GAIN			ADC_GAIN_1_6
#define ADC_REFERENCE		ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40)
#define BUFFER_SIZE			6

static bool _IsInitialized = false;
static uint8_t _LastChannel = 250;
static int16_t m_sample_buffer[BUFFER_SIZE];

static struct adc_channel_cfg m_1st_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = 0, // gets set during init
	.differential	  = 0,
#if CONFIG_ADC_CONFIGURABLE_INPUTS
	.input_positive   = 0, // gets set during init
#endif
};

/* End of Zephyr devices*/



const struct device* cSLIM_adc_init( int channel ) {
	int ret;
	const struct device *adc_dev = device_get_binding(ADC_DEVICE_NAME);
	if(_LastChannel != channel)
	{
		_IsInitialized = false;
		_LastChannel = channel;
	}

	if ( adc_dev != NULL && !_IsInitialized)
	{
		m_1st_channel_cfg.channel_id = channel;
#if CONFIG_ADC_CONFIGURABLE_INPUTS
        m_1st_channel_cfg.input_positive = channel+1,
#endif
		ret = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
		if(ret != 0)
		{
			//LOG_INF("Setting up adc channel failed with code %d", ret);
			adc_dev = NULL;
		}
		else
		{
			_IsInitialized = true;	// we don't have any other analog users
		}
	}
	
	memset(m_sample_buffer, 0, sizeof(m_sample_buffer));
	return adc_dev;
}



static int16_t adc_read_channel(int channel)
{
	const struct adc_sequence sequence = {
		.options     = NULL,				// extra samples and callback
		.channels    = BIT(channel),		// bit mask of channels to read
		.buffer      = m_sample_buffer,		// where to put samples read
		.buffer_size = sizeof(m_sample_buffer),
		.resolution  = ADC_RESOLUTION,		// desired resolution
		.oversampling = 0,					// don't oversample
		.calibrate = 0						// don't calibrate
	};

	int ret;
	int16_t sample_value = BAD_ANALOG_READ;
	const struct device *adc_dev = cSLIM_adc_init(channel);
	if (adc_dev)
	{
		ret = adc_read(adc_dev, &sequence);
		if(ret == 0)
		{
			sample_value = m_sample_buffer[0];
		}
	}

	return sample_value;
}



float adc_read_to_voltage(int adc_read)
{
	int multip = 256;
	// find 2**adc_resolution
	switch(ADC_RESOLUTION)
				
	{
		default :
		case 8 :
			multip = 256;
			break;
		case 10 :
			multip = 1024;
			break;
		case 12 :
			multip = 4096;
			break;
		case 14 :
			multip = 16384;
			break;
	}
	
	float v_out = (adc_read * 3.6 / multip);
	return v_out;
}

double analog_read(analog_type_t type) {
	uint32_t analog_read;
	double voltage;

	switch (type) {
        case BATTERY:
           
		   	analog_read = adc_read_channel(0);
			if(analog_read == BAD_ANALOG_READ)
			{
				LOG_ERR("Bad analog read");
				return analog_read;
			}
			LOG_INF("ADC raw read: %d", analog_read);
			voltage = adc_read_to_voltage(analog_read);
            voltage /= BATTERY_RES2_K;
            voltage *= (BATTERY_RES1_K + BATTERY_RES2_K);
            return voltage;

        case TEMPERATURE:
			//data = <cmd>?AT%XVBAT;
			//err_code = sd_temp_get(&temp);
			//APP_ERROR_CHECK(err_code);
			//temp = (temp * 10) / 4;
			//return (uint16_t) temp;
			return 0;
			break;
        default:
            return 0;
	}
}
