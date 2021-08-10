/*
 * fifo_gen_gen.h
 *
 *  Created on: May 18, 2021
 *      Author: Eivinhj
 */

//TODO: Replace with Zephyr FIFOs

#ifndef FIFO_GEN_H
#define FIFO_GEN_H

#include <stdint.h>
#include <stdbool.h>

#define FIFO_GEN_MAX_SIZE 100

typedef struct {
	uint8_t buffer[FIFO_GEN_MAX_SIZE];
	uint8_t size;
	uint8_t front;
	uint8_t rear;
} fifo_gen_buffer_t;

/**
 * @brief  initiate FIFO buffer
 * @details 
 *
 *  @param[in] buf pointer to buffer
 */
void fifo_gen_init(fifo_gen_buffer_t* buf);

/**
 * @brief Check if buffer is empty
 * @details 
 *
 * @param[in] buf pointer to buffer
 * @retval true if empty
 * @retval false if not empty
 */
bool fifo_gen_is_empty(fifo_gen_buffer_t* buf);

/**
 * @brief Check if buffer is full
 * @details 
 * 
 * @param[in] buf pointer to buffer
 *
 * @retval true if full, false if not full
 */
bool fifo_gen_is_full(fifo_gen_buffer_t* buf);

/**
 * @brief Remove char from buffer
 * @details
 * 
 * @param[in] buf pointer to buffer 
 *
 * @retval character removed
 */
char fifo_gen_remove(fifo_gen_buffer_t* buf);

/**
 * @brief Add char to buffer
 * @details 
 * 
 * @param[in] buf pointer to buffer
 * @param[in] data data to add to buffer
 *
 */
void fifo_gen_add(fifo_gen_buffer_t* buf, char data);

/**
 * @brief Check if buffer contain complete string
 * @details string ends with \r\n or \n\r
 * 
 * @param[in] buf pointer to buffer
 *
 * @retval true if complete string
 * @retval false if not complete string
 */
bool fifo_gen_contain_complete_string(fifo_gen_buffer_t* buf);

/**
 * @brief Check if buffer contain start sign
 * @details 
 * 
 * @param[in] buf pointer to buffer
 * @param[in] start_sign buffer value indicating start of string
 *
 * @retval true on success, false on failiure (string not found)
 */
bool fifo_gen_contain_start_sign(fifo_gen_buffer_t* buf, char start_sign);

/**
 * @brief Get length of buffer
 * @details 
 * 
 * @param[in] buf pointer to buffer 
 *
 * @retval number of bytes filled
 */
uint8_t fifo_gen_get_length(fifo_gen_buffer_t* buf);

#endif //FIFO_GEN_H