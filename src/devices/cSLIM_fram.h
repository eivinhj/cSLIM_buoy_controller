#ifndef CSLIM_FRAM_H
#define CSLIM_FRAM_H

#include <stdint.h>


typedef enum {
	FRAM_WREN	=0x06, //Set write enable latch
	FRAM_WRDI	=0x04, //Reset erite enable latch 
	FRAM_RDSR	=0x05, //Read status register 
	FRAM_WRSR	=0x01, //write status register
	FRAM_WRITE	=0x02, //write memory data
	FRAM_READ	=0x03, //read memory data
	FRAM_FSTRD	=0x0B, //fast read memory data
	FRAM_SSWR	=0x42, //special sector write
	FRAM_SSRD	=0x4B, //special sector read
	FRAM_RDID	=0x9F, //read device id
	FRAM_RUID	=0x4C, //read unique id
	FRAM_WRSN	=0xC2, //write serial number
	FRAM_RDSN	=0xC3, //read serial number
	FRAM_DPD	=0xBA, //enter deep power-down
	FRAM_HBN	=0xB9 //enter hibernate mode
} fram_opcode;



/**
 * @brief Initialize CY15B108QN FRAM
 * @details 
 *
 * @param[in] void		void
 */
void cSLIM_fram_init();


int fram_read_status_register(uint8_t* data);

////
// 8 bit data 
///
/**
 * @brief Read single FRAM address
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to where to store data'
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_read_data(uint32_t address, uint8_t* data);

/**
 * @brief Read multiple FRAM address
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to where to store data
 * 
 * @param[in] data_length number of bytes to read
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_read_data_burst(uint32_t start_address, uint8_t* data, uint8_t data_length);

/**
 * @brief Write single FRAM address
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to data to store
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_write_data(uint32_t address, uint8_t* data);


/**
 * @brief Write multiple FRAM address
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to where to store data'
 * 
 * @param[in] data_length number of bytes to write
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_write_data_burst(uint32_t start_address, uint8_t* data, uint8_t data_length);

////
// 32 bit data 
///



int fram_read_data_32_t(uint32_t address, uint32_t* data);

int fram_read_data_burst_32_t(uint32_t start_address, uint32_t* data, uint8_t data_length);

/**
 * @brief Write single 32 bit value to FRAM 
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to data to store
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_write_data_32_t(uint32_t address, uint32_t* data);


/**
 * @brief Write multiple 32-bit values to FRAM
 * @details FRAM has 1024K adresses of 8bit
 *
 * @param[in] address FRAM adress 
 * 
 * @param[in] data 	pointer to where to store data'
 * 
 * @param[in] data_length number of bytes to write
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int fram_write_data_burst_32_t(uint32_t address, uint32_t* data, uint8_t data_length);

#endif // CSLIM_FRAM_H