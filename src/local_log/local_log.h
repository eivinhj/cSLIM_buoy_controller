#ifndef LOCAL_LOG_H
#define LOCAL_LOG_H

#include <stdint.h>

typedef enum {
	RESERVED,
	EVENT,
	TBR_SENSOR_MESSAGE, 
	TBR_TAG_MESSAGE, 
	CHARSTRING
} LOG_TYPE;

typedef enum {
	TBR_TIME_UPDATE_MISS,
	TBR_TIME_UPDATE_OK,
	GPS_TIME_UPDATE_MISS,
	GPS_TIME_UPDATE_OK,
} EVENT_TYPE;

typedef enum {
	LOG_uSD,
	LOG_FRAM
} LOG_DEVICE_TYPE;

#define MEM_PARTITION_RESERVED_SIZE 4
typedef enum {
	MEM_RESERVED_START 				= 0x00000, //For config etc, not used at the moment
	MEM_RESERVED_END   				= 0x0FFFF,
	MEM_TBR_SENSOR_MESSAGE_START	= 0x10000,
	MEM_TBR_SENSOR_MESSAGE_END		= 0x1FFFF,
	MEM_TBR_TAG_MESSAGE_START  		= 0x20000,
	MEM_TBR_TAG_MESSAGE_END			= 0x9FFFF,
	MEM_EVENT_START					= 0xA0000,	
	MEM_EVENT_END					= 0xAFFFF,
	MEM_CHARSTRING_START			= 0xB0000,
	MEM_CHARSTRING_END				= 0xFFFFF,
} 	FRAM_MEMORY_MAP;



/**
 * @brief Initialize local log
 * @details 
 *
 * @param[in] device_type uSD or FRAM
 * 
 * @retval 0 on success
 */
int local_log_init(LOG_DEVICE_TYPE device_type);

/**
 * @brief Add log message 
 * @details 
 *
 * @param[in] device_type uSD or FRAM
 * 
 * @param[in] log_type Type of message to store
 * 
 * @param[in] data pointer to data bytes to store
 * 
 * @param[in] data_size length of data in number of bytes, only valid for charstring
 * 
 * @retval 0 on success
 */
int local_log_add(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type, char* data, uint16_t data_size);

/**
 * @brief Read log
 * @details 
 *
 * @param[in] device_type uSD or FRAM
 * 
 * @param[in] log_type Type of message to store
 * 
 * @param[in] log_id Log index to read, for charstring: first byte in memory to read
 * 
 * @param[in] data pointer to data bytes to read
 * 
 * @param[in] data_size length of data in number of bytes, only valid for charstring
 * 
 * @retval 0 on success
 */
int local_log_read_log(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type, uint16_t log_id, char* data, uint16_t data_size);

/**
 * @brief 
 * @details 
 *
 * @param[in] device_type uSD or FRAM
 * 
 * @param[in] log_type Type of message to store
 * 
 * @retval number of logged items, for charstring: number of bytes stored in memory
 */
uint16_t local_log_get_log_size(LOG_DEVICE_TYPE device_type, LOG_TYPE log_type);


#endif //LOCAL_LOG_H