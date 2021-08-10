#include "cSLIM_I2C.h"

#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(cSLIM_i2c, LOG_LEVEL_INF);

#define I2C_MAX_BYTES 0x1FFF

/*
 * Zephyr devices
 */
#define I2C_DEV DT_LABEL(DT_ALIAS(i2c2))
const struct device *i2c_dev;

/* End of Zephyr devices*/

int cSLIM_i2c_init()
{
	i2c_dev = device_get_binding(I2C_DEV);
	if (!i2c_dev) {
		LOG_ERR("I2C: Device driver not found.\n");
		return ENODEV;
	}
	return 0;
}
int cSLIM_i2c_write_addr(uint16_t slave_address, uint8_t data_address, uint8_t data)
{
	uint8_t data_w_addr[2];
	data_w_addr[0] = data_address;
	data_w_addr[1] = data;
	return cSLIM_i2c_write_burst(slave_address, 2, data_w_addr);
}
int cSLIM_i2c_write_burst(uint16_t slave_address, uint8_t data_length, const uint8_t* data)
{
	if(data_length >= I2C_MAX_BYTES)
	{
		LOG_ERR("Data buffer is too long");
		return -1;
	}
	return i2c_write(i2c_dev, data, data_length, slave_address);
	//i2c_burst_write(i2c_dev, slave_address, data_address, data, data_length);
}
uint8_t cSLIM_i2c_read(uint16_t slave_address,const  uint8_t data_address)
{
	uint8_t read_buf;
	i2c_write_read(i2c_dev, slave_address, &data_address, 1, &read_buf, 1);
	return read_buf;
}

int cSLIM_i2c_read_burst(uint16_t slave_address,const  uint8_t data_address, uint8_t* read_buffer, uint8_t num_read)
{
	i2c_write_read(i2c_dev, slave_address, &data_address, 1, read_buffer, num_read);
	return 0;
}