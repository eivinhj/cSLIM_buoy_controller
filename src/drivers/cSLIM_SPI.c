/*
 * cSLIM_SPI.c
 *
 * Created: 25.04.21
 *  Author: eivinhj
 */ 

#include "drivers/cSLIM_SPI.h"

#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/spi.h>

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <stdint.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(cSLIM_spi, LOG_LEVEL_INF);



#define SLEEP_TIME_MS	1

/*
 * Zephyr devices
 */
#define CS_GPS_NODE		DT_ALIAS(cs0)
#define CS_SD_NODE		DT_ALIAS(cs1)
#define CS_LORA_NODE	DT_ALIAS(cs2)
#define CS_DISPLAY_NODE	DT_ALIAS(cs3)
#define CS_FRAM_NODE	DT_ALIAS(cs4)


#if DT_NODE_HAS_STATUS(CS_GPS_NODE, okay) && DT_NODE_HAS_PROP(CS_GPS_NODE, gpios)
#define CS_GPS_GPIO_LABEL	DT_GPIO_LABEL(CS_GPS_NODE, gpios)
#define CS_GPS_GPIO_PIN	DT_GPIO_PIN(CS_GPS_NODE, gpios)
#define CS_GPS_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(CS_GPS_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(CS_SD_NODE, okay) && DT_NODE_HAS_PROP(CS_SD_NODE, gpios)
#define CS_SD_GPIO_LABEL	DT_GPIO_LABEL(CS_SD_NODE, gpios)
#define CS_SD_GPIO_PIN	DT_GPIO_PIN(CS_SD_NODE, gpios)
#define CS_SD_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(CS_SD_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(CS_LORA_NODE, okay) && DT_NODE_HAS_PROP(CS_LORA_NODE, gpios)
#define CS_LORA_GPIO_LABEL	DT_GPIO_LABEL(CS_LORA_NODE, gpios)
#define CS_LORA_GPIO_PIN	DT_GPIO_PIN(CS_LORA_NODE, gpios)
#define CS_LORA_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(CS_LORA_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(CS_DISPLAY_NODE, okay) && DT_NODE_HAS_PROP(CS_DISPLAY_NODE, gpios)
#define CS_DISPLAY_GPIO_LABEL	DT_GPIO_LABEL(CS_DISPLAY_NODE, gpios)
#define CS_DISPLAY_GPIO_PIN	DT_GPIO_PIN(CS_DISPLAY_NODE, gpios)
#define CS_DISPLAY_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(CS_DISPLAY_NODE, gpios))
#endif
#if DT_NODE_HAS_STATUS(CS_FRAM_NODE, okay) && DT_NODE_HAS_PROP(CS_FRAM_NODE, gpios)
#define CS_FRAM_GPIO_LABEL	DT_GPIO_LABEL(CS_FRAM_NODE, gpios)
#define CS_FRAM_GPIO_PIN	DT_GPIO_PIN(CS_FRAM_NODE, gpios)
#define CS_FRAM_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(CS_FRAM_NODE, gpios))
#endif

const struct device *cs_gps;
const struct device *cs_sd;
const struct device *cs_lora;
const struct device *cs_display;
const struct device *cs_fram;

/* End of Zephyr devices*/

//SPI Mutex
struct k_mutex spi_mutex;



byte_order_t byte_order;


struct spi_cs_control spi_cs = {
	.gpio_dev 	= NULL,
	.delay 		= 1,
	.gpio_pin	= (uintptr_t) NULL,
	.gpio_dt_flags	= (uintptr_t) NULL,
}; 

static struct spi_config spi_cfg_msb = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.frequency =DT_PROP(DT_NODELABEL(spi3), clock_frequency),
	.slave = 0,
	.cs = &spi_cs,
};
static const struct spi_config spi_cfg_lsb = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_LSB,
	.frequency = DT_PROP(DT_NODELABEL(spi3), clock_frequency),
	.slave = 0,
	.cs = &spi_cs,
};
/*
static const struct spi_config spi_cfg_lsb = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_LSB |
		     SPI_MODE_CPOL | SPI_MODE_CPHA,
	.frequency = DT_PROP(DT_NODELABEL(spi3), clock_frequency),
	.slave = 0,
};*/

	const struct device * spi_dev;
	static uint8_t tx_buffer[1];
	static uint8_t rx_buffer[1];


int cSLIM_spi_get_mutex()
{
	int i = 0; 
	while(k_mutex_lock(&spi_mutex, K_SECONDS(1)))
	{
		LOG_ERR("Could not get SPI mutex");
		i++;
		__ASSERT(i<100, "Could not get SPI mutex in 100 attempts");
	}
	return 0;
}
int cSLIM_spi_release_mutex()
{
	k_mutex_unlock(&spi_mutex);
	return 0; 
}

int cSLIM_spi_init()
{
	k_mutex_init(&spi_mutex);
	char* spiName = "SPI_3";
	spi_dev = device_get_binding(spiName);
	if (spi_dev == NULL) {
		printk("Could not get %s device\n", spiName);
		return 1;
	}
	byte_order = MSBF;
	return 0;
}


int cSLIM_CS_init(device_t device)
{
	int ret = 0;
	switch (device)
	{
	case GPS:
		cs_gps = device_get_binding(CS_GPS_GPIO_LABEL);
		if (cs_gps == NULL) {
		printk("Didn't find LED device %s\n", CS_GPS_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(cs_gps, CS_GPS_GPIO_PIN, CS_GPS_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, CS_GPS_GPIO_LABEL, CS_GPS_GPIO_PIN);
		return ret;
		}
		gpio_pin_set(cs_gps, 	CS_GPS_GPIO_PIN, 	1);	
		break;

	case SD_CARD:
		cs_sd = device_get_binding(CS_SD_GPIO_LABEL);
		if (cs_sd == NULL) {
		printk("Didn't find LED device %s\n", CS_SD_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(cs_sd, CS_SD_GPIO_PIN, CS_SD_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, CS_SD_GPIO_LABEL, CS_SD_GPIO_PIN);
		return ret;
	}
		gpio_pin_set(cs_sd, 	CS_SD_GPIO_PIN, 	1);
		break;

	case LORA:
		cs_lora = device_get_binding(CS_LORA_GPIO_LABEL);
		if (cs_lora == NULL) {
		printk("Didn't find LED device %s\n", CS_LORA_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(cs_lora, CS_LORA_GPIO_PIN, CS_LORA_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, CS_LORA_GPIO_LABEL, CS_LORA_GPIO_PIN);
		return ret;
		}
		gpio_pin_set(cs_lora, 	CS_LORA_GPIO_PIN, 	1);
		break;
	case DISPLAY:
		cs_display = device_get_binding(CS_DISPLAY_GPIO_LABEL);
		if (cs_display == NULL) {
		printk("Didn't find GPIO device %s\n", CS_DISPLAY_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(cs_display, CS_DISPLAY_GPIO_PIN, CS_DISPLAY_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure GPIO device %s pin %d\n",
		       ret, CS_DISPLAY_GPIO_LABEL, CS_DISPLAY_GPIO_PIN);
		return ret;
		}
		gpio_pin_set(cs_display,CS_DISPLAY_GPIO_PIN, 0);
		break;
	case FRAM:
		cs_fram = device_get_binding(CS_FRAM_GPIO_LABEL);
		if (cs_fram == NULL) {
		printk("Didn't find LED device %s\n", CS_FRAM_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(cs_fram, CS_FRAM_GPIO_PIN, CS_FRAM_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, CS_FRAM_GPIO_LABEL, CS_FRAM_GPIO_PIN);
		return ret;
		}
		gpio_pin_set(cs_fram, 	CS_FRAM_GPIO_PIN, 	0);	
		break;
	case ALL:
		cSLIM_CS_init(GPS);
		cSLIM_CS_init(SD_CARD);
		cSLIM_CS_init(LORA);
		cSLIM_CS_init(DISPLAY);
		cSLIM_CS_init(FRAM);
	default:
		break;
	}
	return ret;
}

void cSLIM_spi_cs_select(device_t device)
{
	cSLIM_spi_get_mutex();
	
	switch (device)
	{		
	case GPS:
		gpio_pin_set(cs_gps, 	CS_GPS_GPIO_PIN, 	0);	
		k_sleep(K_USEC(10));
		/*
		spi_cs.gpio_dev 		= cs_gps;
		spi_cs.delay 			= 10;
		spi_cs.gpio_pin			= CS_GPS_GPIO_PIN;
		spi_cs.gpio_dt_flags	= CS_GPS_GPIO_FLAGS;
		*/		
		break;
	case SD_CARD:
		gpio_pin_set(cs_sd, 	CS_SD_GPIO_PIN, 	1);
		/*
		spi_cs.gpio_dev 		= cs_sd;
		spi_cs.delay 			= 10;
		spi_cs.gpio_pin			= CS_SD_GPIO_PIN;
		spi_cs.gpio_dt_flags	= (uint8_t) CS_SD_GPIO_FLAGS;	
		*/	
		break;
	case LORA: 
		//gpio_pin_set(cs_lora, 	CS_LORA_GPIO_PIN, 	0);
		spi_cs.gpio_dev 		= cs_lora;
		spi_cs.delay 			= 10;
		spi_cs.gpio_pin			= CS_LORA_GPIO_PIN;
		spi_cs.gpio_dt_flags	= (uint8_t)CS_LORA_GPIO_FLAGS;		
		break; 
	case DISPLAY:
		//gpio_pin_set(cs_display,CS_DISPLAY_GPIO_PIN,1);
		spi_cs.gpio_dev 		= cs_display;
		spi_cs.delay 			= 10;						//From uBlox datasheet
		spi_cs.gpio_pin			= CS_DISPLAY_GPIO_PIN;
		spi_cs.gpio_dt_flags	= (uint8_t)CS_DISPLAY_GPIO_FLAGS;
		break;
	case FRAM:
		gpio_pin_set(cs_fram,	CS_FRAM_GPIO_PIN,	1);
		//spi_cfg_msb.frequency = 20000000;
		/*
		spi_cs.gpio_dev 		= cs_fram;
		spi_cs.delay 			= 10;
		spi_cs.gpio_pin			= CS_FRAM_GPIO_PIN;
		spi_cs.gpio_dt_flags	= (uint8_t)CS_FRAM_GPIO_FLAGS;	
		*/	
		break;
	default:
		break;
	}
}
void cSLIM_spi_cs_release(device_t device)
{
	switch (device)
	{		
	case GPS:
		gpio_pin_set(cs_gps, 	CS_GPS_GPIO_PIN, 	1);	
		break;
	case FRAM:
		gpio_pin_set(cs_fram, 	CS_FRAM_GPIO_PIN, 	0);	
		spi_cfg_msb.frequency = DT_PROP(DT_NODELABEL(spi3), clock_frequency);
		break;
	case SD_CARD:
		gpio_pin_set(cs_sd, 	CS_SD_GPIO_PIN, 	0);
		spi_cs.gpio_dev 		= (const struct device *) NULL;
		spi_cs.delay 			= 1;
		spi_cs.gpio_pin			= (uintptr_t)NULL;
		spi_cs.gpio_dt_flags	= (uintptr_t)NULL;	
	case LORA: 
	case DISPLAY:
		spi_cs.gpio_dev 		= (const struct device *)NULL;
		spi_cs.delay 			= 1;
		spi_cs.gpio_pin			= (uintptr_t)NULL;
		spi_cs.gpio_dt_flags	= (uintptr_t)NULL;
		break;
	case ALL:
		gpio_pin_set(cs_gps, 	CS_GPS_GPIO_PIN, 	1);
		gpio_pin_set(cs_sd, 	CS_SD_GPIO_PIN, 	0);
		gpio_pin_set(cs_lora, 	CS_LORA_GPIO_PIN, 	0);
		gpio_pin_set(cs_display,CS_DISPLAY_GPIO_PIN,0);
		gpio_pin_set(cs_fram,	CS_FRAM_GPIO_PIN,	0);
		break;
	
	default:
		break;
	}
	cSLIM_spi_release_mutex();
}

void cSLIM_spi_set_byte_order(byte_order_t order)
{
	byte_order = order;
}

int cSLIM_spi_read_write_byte(uint8_t write_character, uint8_t* read_buffer)
{
	int err;
	char tmp;
	if(read_buffer==NULL)
	{
		read_buffer = &tmp;
	}
	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};
	tx_buffer[0] = write_character;
	switch (byte_order)
	{
	case MSBF:
		err = spi_transceive(spi_dev, &spi_cfg_msb, &tx, &rx);
		break;
	case LSBF:
		err = spi_transceive(spi_dev, &spi_cfg_lsb, &tx, &rx);
	default:
		err = 0;
		break;
	} 

	if (err) {
		printk("SPI error: %d\n", err);
	} else {
		/* Connect MISO to MOSI for loopback */
		//printk("TX sent: %x\n", tx_buffer[0]);
		//printk("RX recv: %x\n", rx_buffer[0]);
		//tx_buffer[0]++;
	}	
	*read_buffer = rx_buffer[0];
	return err;
}
void cSLIM_spi_write_byte(uint8_t data)
{
	uint8_t tmp;
	cSLIM_spi_read_write_byte(data, &tmp);
}

void cSLIM_spi_write_bytes(uint8_t* data, uint16_t data_length)
{
		int err;
	const struct spi_buf tx_buf = {
		.buf = data,
		.len = data_length
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	/*struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	}; */
	switch (byte_order)
	{
	case MSBF:
		err = spi_transceive(spi_dev, &spi_cfg_msb, &tx, NULL);
		break;
	case LSBF:
		err = spi_transceive(spi_dev, &spi_cfg_lsb, &tx, NULL);
	default:
		err = 0;
		break;
	} 

	if (err) {
		printk("SPI error: %d\n", err);
	} else {
		/* Connect MISO to MOSI for loopback */
		//printk("TX sent: %x\n", tx_buffer[0]);
		//printk("RX recv: %x\n", rx_buffer[0]);
		//tx_buffer[0]++;
	}	
}


uint8_t cSLIM_spi_read_write_bytes(uint8_t* tx_buffer, uint8_t* rx_buffer, uint8_t data_length)
{
	int err;
	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = data_length,
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	const struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = data_length,
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	}; 
	switch (byte_order)
	{
	case MSBF:
		err = spi_transceive(spi_dev, &spi_cfg_msb, &tx, &rx);
		break;
	case LSBF:
		err = spi_transceive(spi_dev, &spi_cfg_lsb, &tx, &rx);
	default:
		err = 0;
		break;
	} 

	if (err) {
		printk("SPI error: %d\n", err);
	} else {
		/* Connect MISO to MOSI for loopback */
		//printk("TX sent: %x\n", tx_buffer[0]);
		//printk("RX recv: %x\n", rx_buffer[0]);
		//tx_buffer[0]++;
	}	
	return err;
}


uint8_t cSLIM_spi_read_byte(void)
{
	uint8_t tmp;
	cSLIM_spi_read_write_byte(0, &tmp);
	return tmp;
}

void cSLIM_spi_dummy_read_n_byte(uint8_t bytes)
{
	uint8_t tmp;
	for(int i = 0; i < bytes; i++)
	{
		cSLIM_spi_read_write_byte(0, &tmp);
	}
}