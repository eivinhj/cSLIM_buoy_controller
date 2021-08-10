/*
 * tbr.h
 *
 *  Created on: Mar 29, 2017
 *      Author: Waseemh
 * 	Adapted for cSLIM on june 2021
 * 		Author: Eivinhj
 */

#ifndef TBR_H
#define TBR_H

#include "../drivers/cSLIM_rs485.h"
#include <time.h>


#define ARRAY_MESSAGE_SIZE		512       // 512
#define CMD_RX_TX_BUF_SIZE		512        // 256
#define	CMD_TX_BUF_SIZE			16
#define TBR_SENSOR_MSG 			true
#define TBR_DETECION_MSG		false
//#define TBR_BACKOFF_DELAY       15          // Response time from TBR
//#define TBR_BACKOFF_DELAY       18          // Response time from TBR
#define TBR_BACKOFF_DELAY       18            // Response time from TBR

#define DEBUG_TBR true

#ifdef DEBUG_TBR
#define DEBUG_TBR_FLAG 1
#else
#define DEBUG_TBR_FLAG 0
#endif

typedef enum {
	ACK01,
	ACK02
} TBR_ACK_TYPE;

typedef struct {
    //First delimiter
	uint16_t	tbrID;
	//Second delimiter
	uint8_t		timeDiff;
	//3rd delimiter
	uint16_t	millisec;
	//4th delimiter
	uint8_t		CodeType;
	uint16_t	Temperature;
	//5th delimiter
	uint32_t	CodeID;
	uint8_t		Noise;
	//6th delimiter
	uint16_t	CodeData;
	uint8_t		NoiseLP;
	//7th delimiter
	uint8_t     frequency;
	uint8_t     SNR;
} tbr_message_t;

//int timepulse_from_rv3032_init();


/*
 * public functions
 */

/**
 * @brief Initialize TBR
 * @details 
 * 
 */
void 		tbr_init( void );

/**
 * @brief Get serial number of TBR sensor
 * @details 
 *
 * @retval TBR serial number
 */
uint32_t 	get_tbr_serial_number();

/**
 * @brief Send command to TBR
 * @details 
 * 
 * @param[in] tbd_cmd Command to send
 * 
 * @param[in] timestamp UTC value used to synchronise the TBR
 *
 * @retval true on success, false on failiure
 */
bool 		tbr_send_cmd(tbr_cmd_t tbr_cmd, uint64_t timestamp);

#endif // TBR_H