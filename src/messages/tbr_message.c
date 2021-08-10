#include "tbr_message.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>

LOG_MODULE_REGISTER(tbr_message, LOG_LEVEL_INF);


uint8_t tbr_transmitter_protocol_to_uint(tbr_tag_detection_t* tag_msg)
{
	tag_msg->transmitter_protocol_int = 0xff;
	if((strncmp(tag_msg->transmitter_protocol_str,(char *)"R256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r256",4) == 0)){
			tag_msg->transmitter_protocol_int = 00;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str, (char *)"R04K", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r04k", 4) == 0 ) ||
				(strncmp(tag_msg->transmitter_protocol_str, (char *)"R04k", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r04K", 4) == 0 )) {
			tag_msg->transmitter_protocol_int = 01;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str, (char *)"R64K", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r64k", 4) == 0) ||
				(strncmp(tag_msg->transmitter_protocol_str, (char *)"R64k", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r64K", 4) == 0)) {
			tag_msg->transmitter_protocol_int = 02;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str,(char *)"S256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"s256", 4) == 0)) {
			tag_msg->transmitter_protocol_int = 03;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str,(char *)"R01M", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r01m", 4)==0) ||
				(strncmp(tag_msg->transmitter_protocol_str,(char *)"R01m", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"r01M", 4)==0)) {
			tag_msg->transmitter_protocol_int = 04;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str,(char *)"S64K", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"s64k", 4) == 0) ||
				(strncmp(tag_msg->transmitter_protocol_str,(char *)"S64k", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"s64K", 4) == 0)) {
			tag_msg->transmitter_protocol_int = 05;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str,(char *)"HS256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"hs256", 4) == 0) ||
				(strncmp(tag_msg->transmitter_protocol_str,(char *)"Hs256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"hS256", 4) == 0)) {
			tag_msg->transmitter_protocol_int = 06;
		}
		else if((strncmp(tag_msg->transmitter_protocol_str,(char *)"DS256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"ds256", 4) == 0) ||
				(strncmp(tag_msg->transmitter_protocol_str,(char *)"Ds256", 4) == 0) || (strncmp(tag_msg->transmitter_protocol_str, (char *)"dS256", 4) == 0)) {
			tag_msg->transmitter_protocol_int = 07;
		}
		else tag_msg->transmitter_protocol_int = 0xFE;


/*		uint8_t diff_freq = tag_msg->detection_frequency_hz-69;
		if (tag_msg->detection_frequency_hz > 69) {
			tag_msg->transmitter_protocol_int = tag_msg->transmitter_protocol_int+(16*diff_freq);
		}
		else if (tag_msg->detection_frequency_hz<69) {
			tag_msg->transmitter_protocol_int = (uint8_t)(tag_msg->transmitter_protocol_int+(16*(diff_freq-1)));
		}
		else {		//in case of 69 KHz do not change code type
			;
		}*/
	return 0;
}

int tbr_compressed_tag_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	//TBR serial and message type
	compressed_buffer[(*compressed_buffer_byte_index)++] =  tbr_serial_number >> 6;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (tbr_serial_number & 0x3f) << 2 | 0b00;  //TODO Check that 0b00 is right

	//Reference Timestamp
	//find time since unix
	//uint32_t ref_timestamp = difftime(seconds_on_ref_timestamp, 0);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 24) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 16) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >>  8) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp) 		& 0xff;
	return 0;
}

int tbr_compressed_log_msg_make_header(uint16_t tbr_serial_number, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	//TBR serial and message type
	compressed_buffer[(*compressed_buffer_byte_index)] =  tbr_serial_number >> 6;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (tbr_serial_number & 0xfc) << 2 | 0b00; //TODO Check that 0b00 is right

	//Reference Timestamp
	//find time since unix
	//uint32_t ref_timestamp = difftime(seconds_on_ref_timestamp, 0);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 24) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >> 16) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp >>  8) & 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (seconds_on_ref_timestamp) 		& 0xff;
	return 0; 
}



int tbr_compressed_tag_msg_add_tag(tbr_tag_detection_t* tag_detection, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	int16_t sec_since_ref_timestamp = (int16_t) difftime(tag_detection->timestamp_unix, seconds_on_ref_timestamp);
	if(sec_since_ref_timestamp < 0 || sec_since_ref_timestamp > 255)
	{
		LOG_ERR("detection is out of reach for reference timestamp");
		return -EINVAL;
	}
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) sec_since_ref_timestamp;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) tag_detection->transmitter_protocol_int;
	switch (tag_detection->transmitter_protocol_int)
	{
	case 0: //R256
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id);
		break;
	case 1://R04K 
	case 2://R64K
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>8);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>0);
		break;
	case 3: //S256
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)tag_detection->transmitter_id;
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)tag_detection->transmitter_data_value;
		break;
	case 4: //R01M
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>16);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>8);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>0);
		break;
	case 5: //S64K
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>8);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_id>>0);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)tag_detection->transmitter_data_value;
		break;
	case 6: //HS256
	case 7: //DS256
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)tag_detection->transmitter_id;
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_data_value>>8);
		compressed_buffer[(*compressed_buffer_byte_index)++] = (uint8_t)(tag_detection->transmitter_data_value>>0);
		break;
	default:
		LOG_ERR("Unknown transmitter protocol");
		break;
	}

	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (((tag_detection->detection_SNR & 0x3f)<<2) | ((tag_detection->timestamp_ms >> 8)&0b11));
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (tag_detection->timestamp_ms & 0xff);

	return 0;
}

int tbr_compressed_log_msg_add_tag(tbr_log_t* log_msg, time_t seconds_on_ref_timestamp, char* compressed_buffer, uint16_t* compressed_buffer_byte_index)
{
	int16_t sec_since_ref_timestamp = (int16_t) difftime(log_msg->timestamp_unix, seconds_on_ref_timestamp);
	if(sec_since_ref_timestamp < 0 || sec_since_ref_timestamp > 255)
	{
		LOG_ERR("detection is out of reach for reference timestamp");
		return -EINVAL;
	}
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) sec_since_ref_timestamp;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) 0xff;
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (log_msg->temperature >> 8 );
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (log_msg->temperature		);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (log_msg->average_background_noise);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (log_msg->peak_background_noise);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) (log_msg->detection_SNR);
	compressed_buffer[(*compressed_buffer_byte_index)++] =  (uint8_t) 0xcc;									//reserved for upper accuracy limit

	return 0;	
}
