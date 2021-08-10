/*
 * fifo_rs232.c
 *
 *  Created on: Apr 6, 2017
 *      Author: Waseemh
 *  Rewritten to use generic fifo May 19, 2021
 *      Author: Eivinhj
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(rs232_fifo);

#include "fifo_rs232.h"
#include "fifo_gen.h"

fifo_gen_buffer_t rs232_tx_buf;
fifo_gen_buffer_t rs232_rx_buf;

/*
 * public variables
 */

/*
 * public functions
 */

void fifo_rs232_init( void ) {
    fifo_gen_init(&rs232_rx_buf);
    fifo_gen_init(&rs232_tx_buf);
}


bool fifo_rs232_is_empty(fifo_rs232_type_t fifo_type) {
	if (fifo_type == fifo_rx_data){
		return fifo_gen_is_empty(&rs232_rx_buf);
	}
	else {
		return fifo_gen_is_empty(&rs232_tx_buf);
	}
	return false;
}

bool fifo_rs232_is_full(fifo_rs232_type_t fifo_type) {
	if (fifo_type == fifo_rx_data){
		return fifo_gen_is_full(&rs232_rx_buf);
	}
	else {
		return fifo_gen_is_full(&rs232_tx_buf);
	}
	return false;
}


char fifo_rs232_remove(fifo_rs232_type_t fifo_type) {
	char temp_data = 0;
	switch (fifo_type){
        case fifo_rx_data:
            temp_data = fifo_gen_remove(&rs232_rx_buf);
            break;

        case fifo_tx_data:
            temp_data = fifo_gen_remove(&rs232_tx_buf);
            break;
		default:
			temp_data = 0;
			LOG_ERR("Invalid fifo type");
			break;
	}
	return temp_data;
}


void fifo_rs232_add(fifo_rs232_type_t fifo_type, char data) {
	switch (fifo_type) {
        case fifo_rx_data:
            fifo_gen_add(&rs232_rx_buf, data);
            break;

        case fifo_tx_data:
            fifo_gen_add(&rs232_tx_buf, data);
            break;
	}
}
