#include "cSLIM_messages.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>

LOG_MODULE_REGISTER(cslim_messages, LOG_LEVEL_INF);

int cslim_status_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	//TBR serial and message type
	compressed_buffer[(*compressed_buffer_byte_index)++] =  tbr_serial_number >> 6;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (tbr_serial_number & 0xfc) << 2 | 0b01;  //TODO Check that 0b01 is right

	//Reference Timestamp
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 24) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 16) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >>  8) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp) 		& 0xff;
	return 0;
}

int cslim_status_msg_add_msg(cslim_status_t* cslim_status, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	compressed_buffer[(*compressed_buffer_byte_index)++] =  ((cslim_status->battery_status &0x07) << 1) 	| ((cslim_status->air_temperature 	>>  6) & 0b01);
	compressed_buffer[(*compressed_buffer_byte_index)++] = 	((cslim_status->air_temperature&0x3f) << 2) 	| ((cslim_status->longitude 		>> 24) & 0b11);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (cslim_status->longitude  >> 16  ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (cslim_status->longitude  >>  8  ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] = 	(cslim_status->longitude         ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] = 	(cslim_status->pdop & 0x7<<  1   )              | ((cslim_status->longitude 		>> 24) & 0b01);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (cslim_status->latitude   >> 16  ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (cslim_status->latitude   >> 8   ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (cslim_status->latitude          ) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  ((cslim_status->fix&0b11)<< 5)                  | (cslim_status->number_of_tracked_satelittes & 0x1f);
	return 0;
}