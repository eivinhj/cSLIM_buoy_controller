#ifndef cSLIM_I2C_H
#define cSLIM_I2C_H

#include <stdint.h>

/**
 * @brief Initialize i2c
 * @details 
 *
 * @retval 
 */
int cSLIM_i2c_init();

/**
 * @brief write single byte to memory address on i2c device
 * @details 
 *
 * @retval 
 */
int cSLIM_i2c_write_addr(uint16_t slave_address, uint8_t data_address, uint8_t data);

/**
 * @brief Write multiple bytes to memory address on i2c device
 * @details 
 *
 * @retval 
 */
int cSLIM_i2c_write_burst(uint16_t slave_address, uint8_t data_length, const uint8_t* data);

/**
 * @brief Read memory adress on i2c device
 * @details 
 *
 * @retval 
 */
uint8_t cSLIM_i2c_read(uint16_t slave_address, uint8_t data_address);

/**
 * @brief Read successive memory adresses on i2c device
 * @details 
 *
 * @retval 
 */
int cSLIM_i2c_read_burst(uint16_t slave_address,const  uint8_t data_address, uint8_t* read_buffer, uint8_t num_read);
#endif // cSLIM_I2C_H