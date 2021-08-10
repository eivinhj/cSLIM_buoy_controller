/*
 * wlr089u0.h
 *
 *  Created on: Mar 27, 2017
 *      Author: Waseemh
 */

#ifndef SRC_WLR08Uu0_H_
#define SRC_WLR08Uu0_H_

#define ENABLE_WLR089U0 true

#ifdef ENABLE_WLR089U0
#define ENABLE_WLR089U0_FLAG 1
#else
#define ENABLE_WLR089U0_FLAG 0
#endif


#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
 #include <drivers/uart.h> 
 #include "../buffers/fifo_gen.h"


 typedef enum{
	 ok = 0,
	 invalid_param,
	 keys_not_init,
	 no_free_ch,
	 silent,
	 busy,
	 mac_paused, 
	 denied,
	 accepted,
	 not_joined,
	 frame_counter_err_rejoin_needed,
	 invalid_data_len,
	 mac_tx_ok,
	 max_rx,
	 no_ack,
	 mic_error
 } WLR_RESPONSE_CODE;

 typedef enum{
	 EU868,
	 EU433,
	 NA915,
	 KR290,
	 JPN923,
	 AS923,
	 IND865
 } WLR_MAC_BAND;

 typedef enum{
	 STANDBY,
	 BACKUP
 } WLR_SYS_SLEEP_TYPE;

extern const unsigned char  	        wlr089u0_tx_buf_gnss[256];

/**
 * @brief Initiate WLR089u0 communication interface
 * @details including necessary GPIO and FIFOs
 *
 * @retval 0 on success
 * @retval Negative retval on failiure
 */
int 	wlr089u0_init( uart_irq_callback_user_data_t uart_cb);

/**
 * @brief Enable WLR089u0
 * @details 
 *
 * @retval 
 */
void 	wlr089u0_enable( void );

/**
 * @brief Disable WLR089u0
 * @details 
 *
 * @retval 
 */
void 	wlr089u0_disable( void );

/**
 * @brief Transmit string to WLR089u0
 * @details 
 *
 * @retval 
 */
int 	wlr089u0_transmit_string(const unsigned char* data,uint8_t length);

/**
 * @brief Transmit char to WLR089u0
 * @details 
 *
 * @retval 
 */
int 	wlr089u0_transmit_char(unsigned char data );


/**
 * @brief Shutdown WLR089u0
 * @details 
 *
 * @retval 
 */
void 	wlr089u0_shutdown( void );

/**
 * @brief Get pointer to RX buffer
 * @details 
 *
 * @retval 
 */
fifo_gen_buffer_t* get_rx_buffer();

/**
 * @brief Get pointer to TX buffer
 * @details 
 *
 * @retval 
 */
fifo_gen_buffer_t* get_tx_buffer();


#endif /* SRC_WLR08Uu0_H_ */
