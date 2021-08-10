/*
 * local_log.c
 *
 * Created: 25.04.21
 *  Author: eivinhj
 * 
 * NOTE: AS the FRAM chip on prototype is damaged, logging with FRAM has not been tested
 */ 

#include "local_log.h"

#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(local_log, LOG_LEVEL_INF);

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>


#include "../devices/cSLIM_fram.h"
#include "../devices/cSLIM_sd.h"

/*
typedef struct mem_partition {
	FRAM_MEMORY_MAP current_index_address;
	uint32_t 		data_start_address;
	uint32_t 		current_index;
	FRAM_MEMORY_MAP final_address;
	uint16_t		message_size_bytes;
} mem_partition_t;

mem_partition_t fram_tbr_sensor_message_partition = {
	.current_index_address		= MEM_TBR_SENSOR_MESSAGE_START,
	.data_start_address 		= MEM_TBR_SENSOR_MESSAGE_START + MEM_PARTITION_RESERVED_SIZE,
	.current_index  			= 0,
	.final_address 				= MEM_TBR_SENSOR_MESSAGE_END,
	.message_size_bytes 		= 7,
};

mem_partition_t fram_tbr_tag_message_partition = {
	.current_index_address		= MEM_TBR_TAG_MESSAGE_START,
	.data_start_address 		= MEM_TBR_TAG_MESSAGE_START + MEM_PARTITION_RESERVED_SIZE,
	.current_index  			= 0,
	.final_address 				= MEM_TBR_TAG_MESSAGE_END,
	.message_size_bytes 		= 7,
};

mem_partition_t fram_event_partition = {
	.current_index_address		= MEM_EVENT_START,
	.data_start_address 		= MEM_EVENT_START + MEM_PARTITION_RESERVED_SIZE,
	.current_index  			= 0,
	.final_address 				= MEM_EVENT_END,
	.message_size_bytes 		= 3, //event type, unix ts, us ts
};

mem_partition_t fram_charstring_partition = {
	.current_index_address		= MEM_CHARSTRING_START,
	.data_start_address 		= MEM_CHARSTRING_START + MEM_PARTITION_RESERVED_SIZE,
	.current_index  			= 0,
	.final_address 				= MEM_CHARSTRING_END,
	.message_size_bytes 		= 1,
};

*/

int local_log_init(LOG_DEVICE_TYPE device_type)
{
	switch (device_type)
	{
	case LOG_FRAM:
		cSLIM_fram_init();
		//fram_read_data_32_t(fram_tbr_sensor_message_partition.current_index_address, 	&(fram_tbr_sensor_message_partition.current_index	));
		//fram_read_data_32_t(fram_tbr_tag_message_partition.current_index_address, 		&(fram_tbr_tag_message_partition.current_index		));
		//fram_read_data_32_t(fram_event_partition.current_index_address, 				&(fram_event_partition.current_index				));
		//fram_read_data_32_t(fram_charstring_partition.current_index_address, 			&(fram_charstring_partition.current_index			));
		break;
	case LOG_uSD:
		sd_card_init();

		break;
	default:
		break;
	}
	return 0; 
}

int local_log_add(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type, char* data, uint16_t data_size)
{
	//mem_partition_t* mem_partition;
	//int status;
	switch (device_type)
	{
	/*
	case LOG_FRAM:
		switch (log_type)
		{
		case EVENT:
			mem_partition = &fram_event_partition;
			break;
		case TBR_SENSOR_MESSAGE:
			mem_partition = &fram_tbr_sensor_message_partition;
			break; 
		case TBR_TAG_MESSAGE:	
			mem_partition = &fram_tbr_tag_message_partition;
			break;
		case CHARSTRING:	
			mem_partition = &fram_charstring_partition;
			break;		
		default:
			return -ENXIO;
			break;
		}

		if(data_size > mem_partition->message_size_bytes && log_type != CHARSTRING)
		{
			LOG_ERR("data_size to large");
			return -E2BIG;
		} 

		if(((mem_partition->current_index)*(mem_partition->message_size_bytes) + data_size) >=mem_partition->final_address )
		{
			LOG_ERR("Not enough space");
			return -ENOSPC;
		} 

		status = fram_write_data_burst(mem_partition->data_start_address + (mem_partition->current_index)*(mem_partition->message_size_bytes), data, data_size);
		if(data_size < mem_partition->message_size_bytes)
		{	
			//fill remaining part of buffer with zeros
			uint8_t temp_data[8] = {0};
			fram_write_data_burst(mem_partition->data_start_address + (mem_partition->current_index)*(mem_partition->message_size_bytes) + data_size, temp_data, mem_partition->message_size_bytes - data_size);
		}
		
		mem_partition->current_index++;
		fram_write_data_32_t(mem_partition->current_index_address, &(mem_partition->current_index));

		break;
	*/
	case LOG_uSD:
		case LOG_FRAM:
		switch (log_type)
		{
		case EVENT:
			sd_card_write("event", data, data_size);
			break;
		case TBR_SENSOR_MESSAGE:
			sd_card_write("tbrsens", data, data_size);
			break; 
		case TBR_TAG_MESSAGE:	
			sd_card_write("tbrtag", data, data_size);
			break;
		case CHARSTRING:	
			sd_card_write("log", data, data_size);
			break;		
		default:
			return -ENXIO;
			break;
		}
		break;
	default:
		break;
	}
	return 0; 
}

int local_log_add_event(LOG_DEVICE_TYPE device_type, EVENT_TYPE log_type, time_t timestamp_unix, uint32_t timestamp_us)
{
	switch (device_type)
	{
	case LOG_FRAM:
	
		break;
	case LOG_uSD:

		break;
	default:
		break;
	}
	return 0; 
}

int local_log_read_log(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type, uint16_t log_id, char* data, uint16_t data_size)
{
	//mem_partition_t* mem_partition;
	switch (device_type)
	{
	/*
	case LOG_FRAM:
		switch (log_type)
		{
		case EVENT:
			mem_partition = &fram_event_partition;
			break;
		case TBR_SENSOR_MESSAGE:
			mem_partition = &fram_tbr_sensor_message_partition;
			break; 
		case TBR_TAG_MESSAGE:	
			mem_partition = &fram_tbr_tag_message_partition;
			break;
		case CHARSTRING:	
			mem_partition = &fram_charstring_partition;
			break;		
		default:
			return -ENXIO;
			break;
		}
	
		if(log_id > mem_partition->current_index) return -ENODATA;
		return fram_read_data_burst(mem_partition->data_start_address + log_id*(mem_partition->message_size_bytes), data, ((mem_partition->message_size_bytes) * (data_size)));
		break;
	*/
	case LOG_uSD:

		break;
	default:
		break;
	}
	return 0; 
}

uint16_t local_log_get_log_size(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type)
{
	switch (device_type)
	{
	/*
	case LOG_FRAM:
		switch (log_type)
		{
		case EVENT:		
			return fram_event_partition.current_index;
			break;
		case TBR_SENSOR_MESSAGE:
			return fram_tbr_sensor_message_partition.current_index;
			break; 
		case TBR_TAG_MESSAGE:	
			return fram_tbr_tag_message_partition.current_index;
			break;
		case CHARSTRING:	
			return fram_charstring_partition.current_index;
			break;		
		default:
			return 0;
			break;
		}
		break;
	*/
	case LOG_uSD:

		break;
	default:
		break;
	}
	return 0; 
}