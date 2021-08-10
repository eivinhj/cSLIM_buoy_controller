/*
 * fifo_tbr.c
 *
 *  Created on: Mar 29, 2017
 *      Author: Waseemh
 *  Rewritten to use generic fifo May 19, 2021
 *      Author: Eivinhj
 */

#include "fifo_tbr.h"
#include "fifo_gen.h"


/*
 * private variables
 */
fifo_gen_buffer_t tbr_tx_cmd_buf;
fifo_gen_buffer_t tbr_rx_cmd_buf;
fifo_gen_buffer_t tbr_rx_data_buf;
/*
 * public variables
 */

/*
 * public functions
 */

void fifo_tbr_init( void ) {
    fifo_gen_init(&tbr_tx_cmd_buf);
    fifo_gen_init(&tbr_rx_cmd_buf);
    fifo_gen_init(&tbr_rx_data_buf);
}

bool fifo_tbr_is_empty(fifo_tbr_type_t fifo_type) {
	bool temp_flag;
	if (fifo_type == fifo_tbr_rx_data) {
		temp_flag = fifo_gen_is_empty(&tbr_rx_data_buf);
	}
	else if (fifo_type == fifo_tbr_rx_cmd){
        temp_flag = fifo_gen_is_empty(&tbr_rx_cmd_buf);
	}
	else {
        temp_flag = fifo_gen_is_empty(&tbr_tx_cmd_buf);
	}
	return temp_flag;
}


bool fifo_tbr_is_full(fifo_tbr_type_t fifo_type) {
	bool temp_flag;
	if (fifo_type == fifo_tbr_rx_data) {
		temp_flag = fifo_gen_is_full(&tbr_rx_data_buf);
	}
	else if (fifo_type == fifo_tbr_rx_cmd) {
		temp_flag = fifo_gen_is_full(&tbr_rx_cmd_buf);
	}
	else if(fifo_tbr_tx_cmd) {
		temp_flag = fifo_gen_is_full(&tbr_tx_cmd_buf);
	}
	return temp_flag;
}


char fifo_tbr_remove(fifo_tbr_type_t fifo_type){
	char temp_data = 0;
	switch (fifo_type) {
        case fifo_tbr_rx_data:
            temp_data = fifo_gen_remove(&tbr_rx_data_buf);
            break;

        case fifo_tbr_rx_cmd:
            temp_data = fifo_gen_remove(&tbr_rx_cmd_buf);
            break;

        case fifo_tbr_tx_cmd:
             temp_data = fifo_gen_remove(&tbr_tx_cmd_buf);
            break;
	}
	return temp_data;
}


void fifo_tbr_add(fifo_tbr_type_t fifo_type, char data) {
	switch (fifo_type) {
        case fifo_tbr_rx_data:
            fifo_gen_add(&tbr_rx_data_buf, data);
            break;

        case fifo_tbr_rx_cmd:
            fifo_gen_add(&tbr_rx_cmd_buf, data);
            break;

        case fifo_tbr_tx_cmd:
           fifo_gen_add(&tbr_tx_cmd_buf, data);
            break;
	}
}
