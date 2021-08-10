#ifndef TBR_MESSAGE_H
#define TBR_MESSAGE_H

#include <time.h>
#include <stdint.h>

typedef struct {
	uint16_t	tbr_serial_number;
	time_t		timestamp_unix;
	uint32_t	timestamp_ms;
	char*		transmitter_protocol_str;
	uint8_t		transmitter_protocol_int;
	uint32_t	transmitter_id;
	uint32_t	transmitter_data_value;
	uint16_t	detection_SNR;
	uint16_t 	detection_frequency_hz;
	uint32_t	number_of_strings_since_power_up;
} tbr_tag_detection_t;

typedef struct {
	uint16_t	tbr_serial_number;
	time_t		timestamp_unix;
	uint16_t	temperature;
	uint16_t	average_background_noise;
	uint32_t	peak_background_noise;
	uint32_t	detection_SNR;
	uint32_t	number_of_strings_since_power_up;
} tbr_log_t;

/**
 * @brief Make TBR tag message header
 * @details 
 *
 * @retval 
 */
int tbr_compressed_tag_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

/**
 * @brief Make TBR log message header
 * @details 
 *
 * @retval 
 */
int tbr_compressed_log_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

/**
 * @brief Add tag detection to buffer
 * @details 
 *
 * @retval 
 */
int tbr_compressed_tag_msg_add_tag(tbr_tag_detection_t* tag_detection, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

/**
 * @brief Add TBR log to buffer
 * @details 
 *
 * @retval 
 */
int tbr_compressed_log_msg_add_tag(tbr_log_t* log_msg, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index);

/**
 * @brief Convert TBR transmitter protocol to unsigned int for transmission
 * @details 
 *
 * @retval 
 */
uint8_t tbr_transmitter_protocol_to_uint(tbr_tag_detection_t* tag_msg);
#endif // TBR_MESSAGE_H