#ifndef cSLIM_MESSAGES_H
#define cSLIM_MESSAGES_H

#include <time.h>
#include <stdint.h>

typedef struct {
	uint32_t	tbr_serial_number;
	time_t		timestamp_unix;
	uint8_t 	battery_status;	
	uint8_t		air_temperature;  	
	uint32_t 	longitude;
	uint32_t 	latitude;
	uint8_t 	pdop;
	uint32_t 	fix;
	uint32_t 	number_of_tracked_satelittes;
} cslim_status_t;

/**
 * @brief Make header of cSLIM status message in buffer
 * @details 
 *
 * @retval 
 */
int cslim_status_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

/**
 * @brief Add cSLIM status message to buffer
 * @details 
 *
 * @retval 
 */
int cslim_status_msg_add_msg(cslim_status_t* cslim_status, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

#endif // cSLIM_MESSAGES_H