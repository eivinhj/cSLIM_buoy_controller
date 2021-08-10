/*
 * rv-3032-c7.c
 *
 * Created: 10.05.21
 *  Author: eivinhj
 */ 

#include "rv-3032-c7.h"

#include <errno.h>
#include <zephyr.h>
#include <logging/log.h>
#include <assert.h>

LOG_MODULE_REGISTER(rv3032c7);

#include "../drivers/cSLIM_I2C.h"

#define RTC_NINT_NODE	DT_ALIAS(rtcnint)
#define rtc_timepulse_NODE	DT_ALIAS(rtcclkout)


#if DT_NODE_HAS_STATUS(RTC_NINT_NODE, okay)
#define RTC_NINT_GPIO_LABEL	DT_GPIO_LABEL(RTC_NINT_NODE, gpios)
#define RTC_NINT_GPIO_PIN	DT_GPIO_PIN(RTC_NINT_NODE, gpios)
#define RTC_NINT_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(RTC_NINT_NODE, gpios))
#else
#error "Unsupported board: BTN2 devicetree alias is not defined"
#define RTC_NINT_GPIO_LABEL	""
#define RTC_NINT_GPIO_PIN	0
#define RTC_NINT_GPIO_FLAGS	0
#endif

#if DT_NODE_HAS_STATUS(rtc_timepulse_NODE, okay)
#define rtc_timepulse_GPIO_LABEL	DT_GPIO_LABEL(rtc_timepulse_NODE, gpios)
#define rtc_timepulse_GPIO_PIN	DT_GPIO_PIN(rtc_timepulse_NODE, gpios)
#define rtc_timepulse_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(rtc_timepulse_NODE, gpios))
#else
#error "Unsupported board: BTN2 devicetree alias is not defined"
#define rtc_timepulse_GPIO_LABEL	""
#define rtc_timepulse_GPIO_PIN	0
#define rtc_timepulse_GPIO_FLAGS	0
#endif

//////////////////////////////////////////////////////////////
// Private variable declarations
//////////////////////////////////////////////////////////////


const struct device *rtc_nint;
const struct device *rtc_clkout;

static struct gpio_callback rtc_nint_cb_data;

static struct gpio_callback rtc_clkout_cb_data;
///////////////////////////////////////////////////////////////
// Private function declarations
//////////////////////////////////////////////////////////////

/**
 * @brief Convert bcd number to binary format
 * @details 
 *
 * @param[in] bcd		Number in bcd format
 * 
 * @retval Number in binary format
 */
int rv3032_bcd_to_binary(uint8_t bcd);

/**
 * @brief Convert binary number to bcd format
 * @details 
 *
 * @param[in] binary	Number in binary format
 * 
 * @retval Number in bcd format
 */
int rv3032_binary_to_bcd(uint8_t binary);



//////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////
int rv3032_bcd_to_binary(uint8_t bcd)
{       
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

/**
 * @brief Convert bcd number to binary
 * @details 
 *
 * @param[in] void		void
 */
int rv3032_binary_to_bcd(uint8_t binary)
{
	return (((binary/10) << 4) + binary%10);
}

//////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////
int rv3032_init(void)
{ 
	int err = cSLIM_i2c_init();
	if (err)
	{
		LOG_ERR("Could not initialize rv3032");
		return err;
	}


	return 0;
}

int rv3032_register_callbacks(gpio_callback_handler_t rtc_clkout_callback, gpio_callback_handler_t rtc_nint_callback)
{
	int err;
	rtc_nint = device_get_binding(RTC_NINT_GPIO_LABEL);
	if (rtc_nint == NULL) 
	{
		printk("Error: didn't find %s device\n", RTC_NINT_GPIO_LABEL);
		err = 1;
		return err;
	}
	err = gpio_pin_configure(rtc_nint, RTC_NINT_GPIO_PIN, RTC_NINT_GPIO_FLAGS);
	if (err != 0) 
	{
		printk("Error %d: failed to configure %s pin %d\n", err, RTC_NINT_GPIO_LABEL, RTC_NINT_GPIO_PIN);
		return err;
	}
	

	rtc_clkout = device_get_binding(rtc_timepulse_GPIO_LABEL);
	if (rtc_clkout == NULL) 
	{
		printk("Error: didn't find %s device\n", rtc_timepulse_GPIO_LABEL);
		err = 1;
		return err;
	}
	err = gpio_pin_configure(rtc_clkout, rtc_timepulse_GPIO_PIN, rtc_timepulse_GPIO_FLAGS);
	if (err != 0) 
	{
		printk("Error %d: failed to configure %s pin %d\n", err, rtc_timepulse_GPIO_LABEL, rtc_timepulse_GPIO_PIN);
		return err;
	}
	
	err = gpio_pin_interrupt_configure(rtc_clkout,
			rtc_timepulse_GPIO_PIN,
			GPIO_INT_EDGE_RISING);
	if (err != 0) 
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", err, rtc_timepulse_GPIO_LABEL, rtc_timepulse_GPIO_PIN);
		err = 1;
		return err;
	}		

	err = gpio_pin_interrupt_configure(rtc_nint,
			RTC_NINT_GPIO_PIN,
			GPIO_INT_EDGE_TO_ACTIVE);
	if (err != 0) 
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", err, RTC_NINT_GPIO_LABEL, RTC_NINT_GPIO_PIN);
		err = 1;
		return err;
	}
	gpio_init_callback(&rtc_clkout_cb_data, rtc_clkout_callback, BIT(rtc_timepulse_GPIO_PIN));
	err = gpio_add_callback(rtc_clkout, &rtc_clkout_cb_data);
	if(err) return err;

	gpio_init_callback(&rtc_nint_cb_data, rtc_nint_callback, BIT(RTC_NINT_GPIO_PIN));
	err = gpio_add_callback(rtc_nint, &rtc_nint_cb_data);
	return err;
}

int rv3032_set_time(struct tm* time)
{
	int err;
	uint8_t time_buf[4];
	time_buf[0] = RV3032_SEC;
	time_buf[1] = rv3032_binary_to_bcd(time->tm_sec);
	time_buf[2] = rv3032_binary_to_bcd(time->tm_min);
	time_buf[3] = rv3032_binary_to_bcd(time->tm_hour);
	err = cSLIM_i2c_write_burst(RV3032_SLAVE_ADDRESS, 4, time_buf);
	if(err)
	{
		return err;
	}
	return 0;
}

int rv3032_set_date(struct tm* time)
{
	int err;
	uint8_t date_buf[4];
	date_buf[0] = RV3032_DATE;
	date_buf[1] = rv3032_binary_to_bcd(time->tm_mday);
	date_buf[2] = rv3032_binary_to_bcd(time->tm_mon);
	date_buf[3] = rv3032_binary_to_bcd(time->tm_year);
	err = cSLIM_i2c_write_burst(RV3032_SLAVE_ADDRESS, 4, date_buf);
	if(err)
	{
		return err;
	}
	return 0;
}

int rv3032_set_date_and_time(struct tm* time)
{	int err = rv3032_set_time(time);
	if(err)
	{
		LOG_ERR("Could not set time");
		return err;
	}
	err = rv3032_set_date(time);
	if(err) 
	{
		LOG_ERR("Could not set date");
		return err; 
	}
	return 0;
}

struct tm rv3032_get_date_and_time()
{
	struct tm time; 
	uint8_t read_buf[7];
	cSLIM_i2c_read_burst(RV3032_SLAVE_ADDRESS, RV3032_SEC, &(read_buf[0]) , 7);
	time.tm_sec = 	rv3032_bcd_to_binary(read_buf[0]);
	time.tm_min = 	rv3032_bcd_to_binary(read_buf[1]);
	time.tm_hour = 	rv3032_bcd_to_binary(read_buf[2]);
	time.tm_wday = 	rv3032_bcd_to_binary(read_buf[3]);
	time.tm_mday = 	rv3032_bcd_to_binary(read_buf[4]);
	time.tm_mon = 	rv3032_bcd_to_binary(read_buf[5]);
	time.tm_year = 	rv3032_bcd_to_binary(read_buf[6]);
	return time;
}

int rv3032_set_seconds(uint8_t seconds)
{
	return 0;
}



int rv3032_set_clockout_frequency_lf(RV3032_CLKOUT_FREQUENCY frequency)
{
	uint8_t clkout2 = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_CLKOUT2);
	LOG_INF("Clockout2 before setting: %x", clkout2);
	clkout2 &= ~(0b11 << RV3032_CLKOUT2_FD_POS | RV3032_CLKOUT2_OS);
	switch (frequency)
	{
	case F_32768HZ:
		clkout2 |= RV3032_CLKOUT2_FD_32768HZ;
		break;
	case F_1024HZ:
		clkout2 |= RV3032_CLKOUT2_FD_1024HZ;
		break;
	case F_64HZ:
		clkout2 |= RV3032_CLKOUT2_FD_64HZ;
		break;
	case F_1HZ:
		clkout2 |= RV3032_CLKOUT2_FD_1HZ;
		break;
	default:
		break;
	}

	cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_CLKOUT2, clkout2);

	clkout2 = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_CLKOUT2);
	LOG_INF("Clockout2 after setting: %x", clkout2);
	return 0; 
}

int rv3032_get_clockout_frequency_lf()
{
	uint8_t clkout2 = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_CLKOUT2);
	clkout2 = ((clkout2 & 0b11) >> RV3032_CLKOUT2_FD_POS);
	switch (clkout2)
	{
	case F_32768HZ:
		return 32768;
		break;
	case F_1024HZ:
		return 1024;
		break;
	case F_64HZ:
		return 64;
		break;
	case F_1HZ:
		return 1;
		break;
	default:
		break;
	}

	cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_CLKOUT2, clkout2);
	return 0; 
}

int rv3032_clkout_enable()
{
	uint8_t pmu = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_PMU);
	LOG_INF("RV3032 PMU: %x", pmu);
	pmu &= ~(RV3032_PMU_NCLKE);

	cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_PMU, pmu);

	uint8_t pmu2 = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_PMU);
	if(pmu != pmu2)
	{
		LOG_ERR("PMU not set");
		return 1;
	} 
	LOG_INF("RV3032 PMU set to: %x", pmu);
	return 0; 
}

int rv3032_clkout_disable()
{
	uint8_t pmu = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_PMU);
	pmu |= (RV3032_PMU_NCLKE);


	cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_PMU, pmu);

	uint8_t pmu2 = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_PMU);
	if(pmu != pmu2)
	{
		LOG_ERR("PMU not set");
		return 1;
	} 
	return 0;
}

int rv3032_get_temp()
{
	uint8_t temp_msb = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_TEMP_MSB);
	//uint8_t temp_lsb = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_TEMP_LSB);

	int8_t temp = temp_msb;
	if(temp_msb & 0x80)
	{
		temp = -(~temp_msb +1);	//twos comp
	} 
	return temp;
}

int rv3032_enable_evi_clock_reset()
{
	uint8_t evi = RV3032_EVI_CTRL_EHL | RV3032_EVI_CTRL_ESYN; //Enable clockout | rising edge | enable reset on evi pin
	return cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_EVI_CTRL, evi);

}

int rv3032_correct_aging(int32_t rtc_frequency_uhz) //frequency in microHZ
{
	int err = 0;
	int rtc_age_offset = (rtc_frequency_uhz - 1000000) / 0.2384; //From RV3032 datasheet

	if(rtc_age_offset != 0) //minutes_to_next_gps_update == 0 on first update when correction is not made
	{
		LOG_INF("RTC age correction is needed");
		uint8_t offset = cSLIM_i2c_read(RV3032_SLAVE_ADDRESS, RV3032_OFFSET);
		int8_t offset_offset_prev = (offset & 0x3f);
		if(offset_offset_prev & 0x20) //twos complement
		{
			offset_offset_prev -= 64;
		}


		rtc_age_offset += offset_offset_prev;
		if(rtc_age_offset < -32 || rtc_age_offset > 31)
		{
			LOG_ERR("RTC age offset is too large to be corrected, discarding");
			return -1;
		}
		
		if(rtc_age_offset < 0) rtc_age_offset += 64; //Twos complement

		rtc_age_offset = rtc_age_offset + (offset&0xC0); //keep flags as is
		err = cSLIM_i2c_write_addr(RV3032_SLAVE_ADDRESS, RV3032_OFFSET, rtc_age_offset);
	}

	return err;
}