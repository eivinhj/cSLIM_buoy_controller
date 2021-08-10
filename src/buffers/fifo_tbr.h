/*
 * fifo_tbr.h
 *
 *  Created on: Mar 29, 2017
 *      Author: Waseemh
 * 	Edit 2021
 * 		Author: Eivinhj
 */

//TODO: Replace with Zephyr FIFOs

#ifndef SRC_FIFO_TBR_H_
#define SRC_FIFO_TBR_H_

#include <stdlib.h>
#include <stdbool.h>


#define 	FIFO_TBR_RX_DATA_SIZE		1024
#define 	FIFO_TBR_RX_CMD_SIZE		0
#define 	FIFO_TBR_TX_CMD_SIZE		16

typedef enum{
	fifo_tbr_rx_data=0,
	fifo_tbr_rx_cmd,
	fifo_tbr_tx_cmd
} fifo_tbr_type_t;

/**
 * @brief Initiate TBR FIFO
 * @details 
 *
 * @param[in] void		void
 */
void    fifo_tbr_init(void);
/**
 * @brief Add byte to FIFO
 * @details 
 *
 * @param[in] fifo_type 	TODO
 * @param[in] data				TODO
 */
void    fifo_tbr_add(fifo_tbr_type_t fifo_type,char data);
/**
 * @brief Check if FIFO is empty
 * @details 
 *
 * @param[in] fifo_type	TODO
 */
bool    fifo_tbr_is_empty(fifo_tbr_type_t fifo_type);
/**
 * @brief Check if FIFO is full
 * @details 
 *
 * @param[in] fifo_type	TODO
 * 
 */
bool    fifo_tbr_is_full(fifo_tbr_type_t fifo_type);
/**
 * @brief Remove byte from FIFO
 * @details 
 *
 * @param[in] fifo_type	TODO
 */
char    fifo_tbr_remove(fifo_tbr_type_t fifo_type);

#endif /* SRC_FIFO_TBR_H_ */
