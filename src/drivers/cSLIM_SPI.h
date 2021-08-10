#ifndef cSLIM_SPI_H
#define cSLIM_SPI_H

#include <stdint.h>
typedef enum{
	GPS=0,
	SD_CARD,
	LORA,
	DISPLAY,
	FRAM,
	ALL
}device_t;

typedef enum{
	MSBF=0,
	LSBF
}byte_order_t;

/**
 * @brief Initialize SPI
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_spi_init();

/**
 * @brief Get SPI mutex to make sure no other device is interrupting transmission
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_spi_get_mutex();

/**
 * @brief Release SPI mutex
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_spi_release_mutex();

/**
 * @brief Initialize chipselec of SPI device
 * @details 
 *
 * @param[in] device		device
 */
int cSLIM_CS_init(device_t device);

/**
 * @brief Select SPI device
 * @details 
 *
 * @param[in] device		device to select
 */
void cSLIM_spi_cs_select(device_t device);

/**
 * @brief Release SPI device
 * @details 
 *
 * @param[in] device		device to clear selected
 */
void cSLIM_spi_cs_release(device_t device);

/**
 * @brief Set SPI byte order
 * @details lsbf or msbf
 *
 * @param[in] order		LSB or MSB first (LSBF/MSBF)
 */
void cSLIM_spi_set_byte_order(byte_order_t order);

/**
 * @brief Read and write SPI byte
 * @details 
 *
 * @param[in] data		character to send
 * 
 * @retval Character received
 */
int cSLIM_spi_read_write_byte(uint8_t write_buffer, uint8_t* read_buffer);

/**
 * @brief Read and write multiple SPI bytes
 * @details 
 *
 * @param[in]
 * 
 * @retval Character received
 */
uint8_t cSLIM_spi_read_write_bytes(uint8_t* tx_buffer, uint8_t* rx_buffer, uint8_t data_length);

/**
 * @brief Write byte to SPI
 * @details 
 *
 * @param[in] data		character to send
 */
void cSLIM_spi_write_byte(uint8_t data);

/**
 * @brief Write multiple bytes to SPI
 * @details 
 *
 * @param[in] data		character to send
 */
void cSLIM_spi_write_bytes(uint8_t* data, uint16_t data_length);

/**
 * @brief Read byte from SPI
 * @details 
 *
 * @param[in] void		void
 * 
 * @retval 	Character received
 */
uint8_t cSLIM_spi_read_byte(void);

/**
 * @brief Read n bytes from SPI
 * @details 
 *
 * @param[in] bytes		number of dummy bytes to read
 */
void cSLIM_spi_dummy_read_n_byte(uint8_t bytes);


#endif //cSLIM_SPI_H