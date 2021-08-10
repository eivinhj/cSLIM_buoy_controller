
/*
 * cSLIM_SPI.c
 *
 * Created: 25.06.21
 *  Author: eivinhj
 */ 
#include "cSLIM_fram.h"

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(cSLIM_fram, LOG_LEVEL_DBG);

#include "../drivers/cSLIM_SPI.h"

//////////////////////////////////
// Public functions
//////////////////////////////////
void cSLIM_fram_init()
{
	//cSLIM_spi_init();
	//cSLIM_CS_init(FRAM);

}

#define FRAM_BUFFER_SIZE 260


//////////////////////////////////////////////////
// Local function declarations
//////////////////////////////////////////////////
int fram_sleep();
int fram_wakeup();

//////////////////////////////////////////////////
// Local functions
//////////////////////////////////////////////////

int fram_sleep()
{
	int err = 0;

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_HBN);

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 2);

	cSLIM_spi_cs_release(FRAM);

	fram_sleep();
	return err; 
}

int fram_wakeup()
{
	int err = 0;

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (0);

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 1);

	cSLIM_spi_cs_release(FRAM);

	k_sleep(K_USEC(500)); //wait for device to wake up


	return err; 
}

//////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////

int fram_read_status_register(uint8_t* data)
{
	int err = 0;

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_RDSR);

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 2);

	*data = rx_buffer[1];

	cSLIM_spi_cs_release(FRAM);


	return err; 
}

int fram_read_data(uint32_t address, uint8_t* data)
{
	int err = 0;
	err = fram_wakeup();
	char tmp; 
	fram_read_status_register(&tmp);
	if(tmp != 64)
	{
		LOG_ERR("FRAM not responding");
		//return -EHOSTDOWN;
	}

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_READ);
	tx_buffer[1] = ((address >> 16) 	& 0x0f);
	tx_buffer[2] = ((address >> 8)  	& 0xff);
	tx_buffer[3] = (address 	 	 	& 0xff);

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 5);

	*data = rx_buffer[4];

	cSLIM_spi_cs_release(FRAM);

	fram_sleep();


	return err; 
}

int fram_read_data_burst(uint32_t start_address, uint8_t* data, uint8_t data_length)
{
	int err = 0;
	err = fram_wakeup();
	char tmp; 
	fram_read_status_register(&tmp);
	if(tmp != 64)
	{
		LOG_ERR("FRAM not responding");
		//return -EHOSTDOWN;
	}

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_READ);
	tx_buffer[1] = ((start_address >> 16) 	& 0x0f);
	tx_buffer[2] = ((start_address >> 8)  	& 0xff);
	tx_buffer[3] = (start_address 	 	 	& 0xff);

	err = cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 4 + data_length) ; //4 + data_length

	for (int i = 0; i < data_length; i++)
	{		
		data[i] = rx_buffer[i+4]; 
	}


	cSLIM_spi_cs_release(FRAM);

	fram_sleep();


	return err; 
}

int fram_write_data(uint32_t address, uint8_t* data)
{
	int err = 0;

	err = fram_wakeup();
	
	char tmp; 
	fram_read_status_register(&tmp);
	if(tmp != 64)
	{
		LOG_ERR("FRAM not responding");
		//return -EHOSTDOWN;
	}


	if(address >= 0x100000)
	{
		LOG_ERR("Adress %d is out of range", address);
		return -EFAULT;
	}

	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WREN);
	cSLIM_spi_cs_release(FRAM);

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_WRITE);
	tx_buffer[1] = ((address >> 16) 	& 0x0f);
	tx_buffer[2] = ((address >> 8)  	& 0xff);
	tx_buffer[3] = (address 	 	 	& 0xff);
	tx_buffer[4] = *data;

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, 5);

	cSLIM_spi_cs_release(FRAM);

	fram_sleep();

	return err; 
}

int fram_write_data_burst(uint32_t start_address, uint8_t* data, uint8_t data_length)
{
	int err = 0;
	err = fram_wakeup();
	char tmp; 
	fram_read_status_register(&tmp);
	if(tmp != 64)
	{
		LOG_ERR("FRAM not responding");
		//return -EHOSTDOWN;
	}

	if(start_address >= 0x100000)
	{
		LOG_ERR("Address %d is out of range", start_address);
		return -EFAULT;
	}

	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WREN);
	cSLIM_spi_cs_release(FRAM);

	uint8_t tx_buffer[FRAM_BUFFER_SIZE] = {0};
	uint8_t rx_buffer[FRAM_BUFFER_SIZE] = {0};

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	tx_buffer[0] = (FRAM_WRITE);
	tx_buffer[1] = ((start_address >> 16) 	& 0x0f);
	tx_buffer[2] = ((start_address >> 8)  	& 0xff);
	tx_buffer[3] = (start_address 	 	 	& 0xff);

	for (int i = 0; i < data_length; i++)
	{
		tx_buffer[i+4] = data[i];
	}

	cSLIM_spi_read_write_bytes(tx_buffer, rx_buffer, data_length + 4);

	cSLIM_spi_cs_release(FRAM);

	fram_sleep();


	return err; 
}

//////////////
//TODO BELOW CHANGE TO write bytes / read bytes spi

int fram_read_data_32_t(uint32_t address, uint32_t* data)
{
	int err = 0;

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_READ);
	cSLIM_spi_write_byte((address >> 16) 	& 0x0f);
	cSLIM_spi_write_byte((address >> 8)  	& 0xff);
	cSLIM_spi_write_byte( address 			& 0xff);

	uint8_t tmp_data[4];

	for (int i = 0; i < 4; i++)
	{
			tmp_data[i] = cSLIM_spi_read_byte();
	}
	cSLIM_spi_cs_release(FRAM);

	*data = (tmp_data[0] << 24) | (tmp_data[1] << 16) | (tmp_data[2] << 8) | (tmp_data[3]);


	return err; 
}

int fram_read_data_burst_32_t(uint32_t start_address, uint32_t* data, uint8_t data_length)
{
	int err = 0;

	cSLIM_spi_set_byte_order(MSBF);
	
	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_READ);
	cSLIM_spi_write_byte((start_address >> 16) 	& 0x0f);
	cSLIM_spi_write_byte((start_address >> 8)  	& 0xff);
	cSLIM_spi_write_byte(start_address 			& 0xff);

	uint8_t tmp_data[4];
	for (int dl = 0; dl < data_length; dl++)
	{
		for (int i = 0; i < 4; i++)
		{
				tmp_data[i] = cSLIM_spi_read_byte();
		}


		data[dl] = (tmp_data[0] << 24) | (tmp_data[1] << 16) | (tmp_data[2] << 8) | (tmp_data[3]);
	}

	cSLIM_spi_cs_release(FRAM);


	return err; 
}

int fram_write_data_32_t(uint32_t address, uint32_t* data)
{
	int err = 0;

	if(address >= 0x100000)
	{
		LOG_ERR("Adress %d is out of range", address);
		return -EFAULT;
	}

	cSLIM_spi_set_byte_order(MSBF);
	
	//Enable write mode
	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WREN);
	cSLIM_spi_cs_release(FRAM);

	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WRITE);
	cSLIM_spi_write_byte((address >> 16) 	& 0x0f);
	cSLIM_spi_write_byte((address >> 8)  	& 0xff);
	cSLIM_spi_write_byte(address 			& 0xff);

	cSLIM_spi_write_byte(((*data) >> 24) 	& 0xff);
	cSLIM_spi_write_byte(((*data) >> 16) 	& 0xff);
	cSLIM_spi_write_byte(((*data) >>  8) 	& 0xff);
	cSLIM_spi_write_byte((*data) 		 	& 0xff);

	cSLIM_spi_cs_release(FRAM);

	return err; 
}


int fram_write_data_burst_32_t(uint32_t address, uint32_t* data, uint8_t data_length)
{
	int err = 0;

	if(address >= 0x100000)
	{
		LOG_ERR("Adress %d is out of range", address);
		return -EFAULT;
	}

	cSLIM_spi_set_byte_order(MSBF);
	
	//Enable write mode
	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WREN);
	cSLIM_spi_cs_release(FRAM);

	cSLIM_spi_cs_select(FRAM);
	cSLIM_spi_write_byte(FRAM_WRITE);
	cSLIM_spi_write_byte((address >> 16) 	& 0x0f);
	cSLIM_spi_write_byte((address >> 8)  	& 0xff);
	cSLIM_spi_write_byte(address 			& 0xff);

	uint32_t tmp_data;
	for (int i = 0; i < data_length; i++)
	{
		tmp_data = data[i];
		cSLIM_spi_write_byte(((tmp_data) >> 24) & 0xff);
		cSLIM_spi_write_byte(((tmp_data) >> 16) & 0xff);
		cSLIM_spi_write_byte(((tmp_data) >>  8) & 0xff);
		cSLIM_spi_write_byte(( tmp_data) 		& 0xff);
	}
	cSLIM_spi_cs_release(FRAM);


	cSLIM_spi_cs_release(FRAM);

	return err; 
}