/*
 * rs232.h
 *
 *  Created on: Mar 27, 2017
 *      Author: Waseemh
 */

#ifndef SRC_RS232_H_
#define SRC_RS232_H_

#define ENABLE_RS232 false

#if ENABLE_RS232
#define ENABLE_RS232_FLAG 1
#else
#define ENABLE_RS232_FLAG 0
#endif



#include "../buffers/fifo_rs232.h"
#include <stdint.h>
#include <stdlib.h>

extern const unsigned char  	        rs232_tx_buf_gnss[256];

//TODO comment this, is quite equal to RS485 interface

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
int 	rs232_init( void );

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
void 	rs232_enable( void );

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
void 	rs232_disable( void );

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
int 	rs232_transmit_string(const unsigned char* data,uint8_t length);

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
int 	rs232_transmit_char(unsigned char data );

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
char 	rs232_receive( void );			//not required in current scenario;implemented as blocking, change to INT if required

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
void 	rs232_reset( void );

/**
 * @brief 
 * @details 
 *
 * @retval 
 */
void 	rs232_shutdown( void );

#endif /* SRC_RS232_H_ */
