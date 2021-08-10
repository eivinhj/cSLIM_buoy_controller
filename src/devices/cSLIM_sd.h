#ifndef cSLIM_SD_H
#define  cSLIM_SD_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize SD card interface
 * @details 
 *
 * @param[in] void		void
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */

int	sd_card_init( void );
/**
 * @brief Turn SD card on
 * @details 
 *
 * @param[in] void		void
 *
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */

int 	sd_card_on( void );

/**
 * @brief Turn SD card off
 * @details 
 *
 * @param[in] void		void
 */
void 	sd_card_off( void );

/**
 * @brief Write to SD card
 * @details 
 *
 * @param[in] file_name File name to write to
 * 
 * @param[in] write_buf pointer to buffer containing data to write
 * 
 * @param[in] num_bytes number of bytes to write
 * 
 * @retval 0 success 
 * 	
 * @retval Negative retval on error
 */
int 	sd_card_write(const char* file_name, char *write_buf, uint32_t num_bytes);


/**
 * @brief [TODO, not implemented] Read from SD card
 * @details 
 *
 * @param[in] file_name File name to read from
 * 
 * @param[in] read_buf pointer to buffer to write to
 * 
 * @param[in] num_bytes number of bytes to read
 * 
 * @retval 0 success
 * 	
 * @retval Negative retval on error
 */
int 	sd_card_read(const char* file_name, char *read_buf, uint32_t num_bytes);

#endif // cSLIM_SD_H