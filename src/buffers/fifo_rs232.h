/*
 * fifo_rs232.h
 *
 *  Created on: Apr 6, 2017
 *      Author: Waseemh
 * 	Edit 2021:
 * 		Author: Eivinhj
 */

//TODO: Replace with Zephyr FIFOs
#ifndef SRC_FIFO_RS232_H_
#define SRC_FIFO_RS232_H_

#include <stdlib.h>
#include <stdbool.h>

#define FIFO_TX_RS232_SIZE		512
#define FIFO_RX_RS232_SIZE		16

typedef enum {
	fifo_rx_data=0,
	fifo_tx_data
} fifo_rs232_type_t;

/**
 * @brief Initiate RS232 fifo
 * @details 
 *
 * @retval 
 */
void    fifo_rs232_init(void);

/**
 * @brief Check if FIFO is empty
 * @details 
 *
 * @retval 
 */
bool    fifo_rs232_is_empty(fifo_rs232_type_t fifo_type);

/**
 * @brief Check if FIFO is full
 * @details 
 *
 * @retval 
 */
bool    fifo_rs232_is_full(fifo_rs232_type_t fifo_type);

/**
 * @brief Remove byte from FIFO
 * @details 
 *
 * @retval 
 */
char    fifo_rs232_remove(fifo_rs232_type_t fifo_type);

/**
 * @brief Add byte to FIFO
 * @details 
 *
 * @retval 
 */
void    fifo_rs232_add(fifo_rs232_type_t fifo_type, char data);


#endif /* SRC_FIFO_RS232_H_ */
