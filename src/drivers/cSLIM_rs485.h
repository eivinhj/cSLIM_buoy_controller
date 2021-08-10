#ifndef cSLIM_RS485_H
#define cSLIM_RS485_H


#include "../buffers/fifo_tbr.h"

typedef enum{
	SET,
	BEING_HANDLED,
	NORMAL
} tbr_tx_char;


typedef enum{
	cmd_sn_req=0,
	cmd_basic_sync,
	cmd_advance_sync
}tbr_cmd_t;

/**
 * @brief Initialize RS485 interface
 * @details 
 *
 * @param[in] void		void
 */
int 	rs485_init( void );

/**
 * @brief Enable RS485 Interface
 * @details 
 *
 * @param[in] void		void
 */
void 	rs485_enable( void );

/**
 * @brief Disable RS485 Interface
 * @details 
 *
 * @param[in] void		void
 */
void 	rs485_disable( void );

/**
 * @brief Reset RS485 Interface
 * @details 
 *
 * @param[in] void		void
 */
void 	rs485_reset( void );

/**
 * @brief Transmit Characters over RS485
 * @details 1 ms between each character
 *
 * @param[in] data*		Charstring to transmit
 * @param[in] length	Length of charstring
 */
int rs485_transmit_string(const unsigned char* data, uint8_t length, bool sync_local_time);

/**
 * @brief Transmit Character over RS485
 * @details 1 ms between each character
 *
 * @param[in] data		Character to transmit
 */
int rs485_transmit_char(uint8_t data);

/**
 * @brief Transmit Character over RS485 synchronized
 * @details Send character synchronized on 10 second interval of time_local within +/- 500us
 *
 * @param[in] data		Character to transmit
 */
int rs485_transmit_char_synchronized(uint8_t data);

/**
 * @brief Receive Character over RS485
 * @details 1 ms between each character
 *
 * @param[in] void		void
 */
bool rs485_receive_char(uint8_t* c);

/**
 * @brief Initialize RS485 interface
 * @details 
 *
 * @param[in] void		void
 */
void 	rs485_tx_mode( void );

/**
 * @brief Initialize RS485 interface
 * @details 
 *
 * @param[in] void		void
 */
void 	rs485_rx_mode( void );

/**
 * @brief Set RS485 interface in low power mode 
 * @details Sets low power mode if flag is set to true
 *
 * @param[in] flag		flag
 */
void    rs485_low_power_mode( bool flag );

void rs485_flush_rx_buffer();
#endif //cSLIM_RS485_H