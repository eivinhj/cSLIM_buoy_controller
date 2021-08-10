#include <zephyr.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_dppi.h>

#include <string.h>
#include <stdio.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(tbr, LOG_LEVEL_INF);

#include "tbr.h"


#define PUBLISH_EVENT_INDEX  (0)
#define SUBSCRIBE_TASK_INDEX (1)
#define DPPI_CHANNEL         (0)

/** @brief Set bit at given position. */
#define DPPI_BIT_SET(pos) (1uL << (pos))

/////////////////////////////////////////////////
// private variables
/////////////////////////////////////////////////

static uint32_t tbr_serial_number = 0; 
/////////////////////////////////////////////////
// private function declarations
/////////////////////////////////////////////////

static bool 		get_and_compare(char *compare_string, char* cmd_rx_tx_buf);
static uint8_t 	CalculateLuhn(time_t * time);
static bool 		get_and_check_ack(TBR_ACK_TYPE ack, bool allow_partial_ack);

/////////////////////////////////////////////////
// Public functions 
/////////////////////////////////////////////////

void tbr_init(void) {
	//GPIO_PinModeSet(PWR_EN_PORT, RS485_12V_PWR_EN, gpioModePushPull, 0);
	rs485_init();
	rs485_enable();

}

int timepulse_from_rv3032_init()  //configure forwarding of timepulse message, only valid for new TBR Live, not implemented or tested
{
/*
	// Configure GPIOTE Index 0 to be an Event 
    nrf_gpiote_event_configure(PUBLISH_EVENT_INDEX,SW0_GPIO_PIN,NRF_GPIOTE_POLARITY_HITOLO);

    // Configure GPIOTE Index 1 to be a Task
    nrf_gpiote_task_configure(SUBSCRIBE_TASK_INDEX,LED3_GPIO_PIN,NRF_GPIOTE_POLARITY_TOGGLE,NRF_GPIOTE_INITIAL_VALUE_LOW);

    // Index 0 will Publish on DPPI Channel 0
    nrf_gpiote_publish_set(NRF_GPIOTE_EVENTS_IN_0,DPPI_CHANNEL);

    // Index 1 will Subscribe on DPPI Channel 0 
    nrf_gpiote_subscribe_set(NRF_GPIOTE_TASKS_OUT_1,DPPI_CHANNEL);

    // Enable Publish and Subscribe 
    nrf_gpiote_event_enable(PUBLISH_EVENT_INDEX);
    nrf_gpiote_task_enable(SUBSCRIBE_TASK_INDEX);

    // Enable DPPI Channel 
    nrf_dppi_channels_enable(NRF_DPPIC, DPPI_BIT_SET(DPPI_CHANNEL));

    // Validate things did get enabled 
    printk("[0] is %d\n",nrf_gpiote_te_is_enabled(PUBLISH_EVENT_INDEX));
    printk("[1] is %d\n",nrf_gpiote_te_is_enabled(SUBSCRIBE_TASK_INDEX));
    printk("[2] is %d\n",nrf_dppi_channel_check(NRF_DPPIC,DPPI_CHANNEL));
*/
return 0;
}

bool tbr_send_cmd(tbr_cmd_t tbr_cmd, uint64_t timestamp) {
	char 		cmd_tx_buf[CMD_TX_BUF_SIZE];
	bool		ret_flag = false;
	uint64_t		my_timestamp;
	uint8_t		luhn;
	//int			temp_var = 0;

	//Flush rs485 rx buffer
	rs485_flush_rx_buffer();

	if (tbr_cmd == cmd_sn_req){
		sprintf((char *)cmd_tx_buf, "?\n");
		rs485_transmit_string(cmd_tx_buf, 1, false);
		char    cmd_rx_tx_buf[CMD_RX_TX_BUF_SIZE] = {0};
		ret_flag = get_and_compare((char *)"SN=", cmd_rx_tx_buf);
		if(ret_flag)
		{
			char* serial_num = &(cmd_rx_tx_buf[3]);
			//remove \r

			tbr_serial_number=atoi(strtok(serial_num, "\r"));
		}
	}
	else if(tbr_cmd == cmd_basic_sync){
		LOG_DBG("Basic sync");
		//sprintf((char *)cmd_tx_buf,"(+)\n");
		cmd_tx_buf[0] = '(';
		cmd_tx_buf[1] = '+';
		cmd_tx_buf[2] = ')';
		cmd_tx_buf[2] = '\r';
		rs485_transmit_string(cmd_tx_buf, 3, true);
		ret_flag = get_and_check_ack(ACK01, true);		
	

        if (!ret_flag) 
		{
			LOG_DBG("TBR ack01 NOT received\n");
		}

	}
	else if (tbr_cmd == cmd_advance_sync) {
		LOG_DBG("Advance sync");
		ret_flag = get_and_check_ack(ACK01, true);
		
		my_timestamp = timestamp;
		luhn=CalculateLuhn(&my_timestamp);
		my_timestamp = (my_timestamp/10) * 10 + luhn;
		sprintf((char *)cmd_tx_buf, "(+)%lld", my_timestamp);
		rs485_transmit_string(cmd_tx_buf, 13, true);  //13 to include luhn

		ret_flag = get_and_check_ack(ACK01, true);	
		ret_flag = get_and_check_ack(ACK02, true);	//discart first ack, only second is important
	}
	else ret_flag = false;

	if(ret_flag)
	{
		LOG_DBG("Received ack");
	}
	else
	{
		LOG_DBG("Did not receive ack");
	}
	return ret_flag;
}

uint32_t get_tbr_serial_number()
{
	//tbr_send_cmd(cmd_sn_req, (time_t) 0);  //assuming this has been called somewhere else (tbr_sync_task)
	return tbr_serial_number;
}

/////////////////////////////////////////////////
// private functions
/////////////////////////////////////////////////

static uint8_t CalculateLuhn(time_t * time) {
	int digit, counter;
	uint8_t timeStampArray[9];
	uint32_t luhn_sum = 0;

	time_t timestamp = *time / 10; /* Cut last digit */

	/* Make an array of digits of the time stamp */
	for (counter = 9; counter >= 1; --counter){
		digit = timestamp % 10;
		timestamp = timestamp / 10;
		timeStampArray[counter - 1] = digit;
	}

	/* Calculate luhn sum */
	for (int i = 0; i < 9; ++i) {
		if (i % 2 == 0) timeStampArray[i] += timeStampArray[i];
		if (timeStampArray[i] >= 10) luhn_sum += timeStampArray[i] % 10 + 1;
		else luhn_sum += timeStampArray[i];
	}

	return (luhn_sum * 9) % 10;
}



/*
static int parse_message_tbr(char *buffer) {
	int				loop_var = 0;
	int				token_length = 0;
	char			*token_str;
	char			ref_token[2] = "\r";
	static 	char 	broken_msg_buf[64];
	static 	bool	last_broken_message_flag = false;
	static 	uint8_t	last_broken_message_size = 0;

	uint32_t		inner_loop_var = 0;
	uint32_t		outer_loop_var = 0;
	uint8_t			n_complete_messages = 0;
	uint8_t			n_converted_tokens = 0;
	bool			found_partial_msg = false;

	if (last_broken_message_flag == true) {
		shift_Elements(buffer, CMD_RX_TX_BUF_SIZE, last_broken_message_size);
		for (loop_var = 0; loop_var < last_broken_message_size; ++loop_var) {
			buffer[loop_var] = broken_msg_buf[loop_var];
		}
		last_broken_message_flag = false;
		last_broken_message_size = 0;
	}

	n_complete_messages = 0;
	found_partial_msg = false;
	for (outer_loop_var = 0; outer_loop_var < strlen(buffer); ++outer_loop_var) {
		if (buffer[outer_loop_var] == '$' || buffer[outer_loop_var] == 'a'){
			for (inner_loop_var=1; inner_loop_var < 64; ++inner_loop_var){		//64 is also danger!!!
				if (buffer[outer_loop_var + inner_loop_var] == '\r'){
					++n_complete_messages;
					outer_loop_var += inner_loop_var;
					break;
				}
				if(inner_loop_var == 63) found_partial_msg=true;
			}
		}
		if (found_partial_msg == true) break;
	}

	last_broken_message_size = 0;
	last_broken_message_flag = false;
	clear_buffer(broken_msg_buf, 64);
	if (outer_loop_var < strlen(buffer)) {
		for(loop_var = outer_loop_var; loop_var < strlen(buffer); ++loop_var) {
			broken_msg_buf[loop_var-outer_loop_var] = buffer[loop_var];
		}
		last_broken_message_flag = true;
		last_broken_message_size = loop_var-outer_loop_var;
		 //LOG_DBG("\t\t\t\tParse found and added half message...");
		 //LOG_DBG("%s",broken_msg_buf);

	}
	n_converted_tokens = 0;

	if (n_complete_messages > 0) {
		token_str = strtok(buffer, ref_token);  //originally strtok_r(buffer, ref_token, &save); 
		if (token_str == NULL) {
			LOG_DBG("Parse DANGER, complete messages >0 BUT no token found!!!!\n");
			return -1;
		}
		else {
			if (!array_is_full()) {
				while (token_str != NULL) {
					if (token_str[0] == '$') {
						token_length = strlen(token_str);
						if (token_length > 50) {
							LOG_DBG("Parse DANGER, string length larger than 50!!!");
							LOG_DBG("%s",token_str);
						}
						for (loop_var=0; loop_var < token_length; ++loop_var){
							array_add(token_str[loop_var]);
						}
						array_add('\n');
					}
					else{
						 //LOG_DBG("\t\t\t!!!Parse discarding unknown message type!!!");
						 //LOG_DBG("%s",token_str);
					}
					++n_converted_tokens;
					if (n_converted_tokens >= n_complete_messages) break;
					token_str = strtok(NULL, ref_token);  //originally strtok_r
				}
				return 1;
			}
			else return -1;
		}
	}
	else return 0;
}
*/

static bool get_and_compare(char *compare_string, char* cmd_rx_tx_buf) {
	char    *cmd_compare_str;
	int	    loop_var  = 0;
	char    temp_char = '0';
	bool    ret_flag  = false;

	k_sleep(K_MSEC(TBR_BACKOFF_DELAY));	        // Response time from TBR

	for(loop_var=0; loop_var < FIFO_TBR_RX_DATA_SIZE; ++loop_var) {
		rs485_receive_char(&temp_char);
		if(temp_char == '@') break;
		cmd_rx_tx_buf[loop_var] = temp_char;
	}
	cmd_compare_str = strstr(cmd_rx_tx_buf, (const char *)compare_string);
	if (cmd_compare_str != NULL) ret_flag = true;
    else ret_flag = false;

    bool incomplete_ack_flag = false;

    if( !ret_flag && incomplete_ack_flag ) ret_flag = true;
	
	return ret_flag;
}


static bool get_and_check_ack(TBR_ACK_TYPE ack, bool allow_partial_ack) {
	char    cmd_rx_tx_buf[CMD_RX_TX_BUF_SIZE] = {0};
	char    temp_char = '0';
	bool    ret_flag  = false;


	k_sleep(K_MSEC(TBR_BACKOFF_DELAY));	        // Response time from TBR

	int loop_var = 0;
	//receive until getting end of message or no char to receive
	for(loop_var=0; loop_var < FIFO_TBR_RX_DATA_SIZE; loop_var++) {
		rs485_receive_char(&temp_char);
		if(temp_char == '@') break;
		if(temp_char == '\r') break;
		cmd_rx_tx_buf[loop_var] = temp_char;
	}
	cmd_rx_tx_buf[loop_var] = '\0';
	switch (ack)
	{
	case ACK01:
		ret_flag = (strcmp(cmd_rx_tx_buf, "ack01") == 0) ? true:false;
		break;
	case ACK02:
		ret_flag = (strcmp(cmd_rx_tx_buf, "ack02") == 0) ? true:false;
		break;
	default:
		ret_flag = false;		
		break;
	}

	if(allow_partial_ack && !ret_flag)
	{
		switch (ack)
		{
		case ACK01:
			ret_flag = (strcmp(cmd_rx_tx_buf, "ck01") == 0) ? true:false;
			if(ret_flag) break;
			ret_flag = (strcmp(cmd_rx_tx_buf, "k01") == 0) ? true:false;
			if(ret_flag) break;
			break;
		case ACK02:
			ret_flag = (strcmp(cmd_rx_tx_buf, "ck02") == 0) ? true:false;
			if(ret_flag) break;
			ret_flag = (strcmp(cmd_rx_tx_buf, "k02") == 0) ? true:false;
			if(ret_flag) break;
			break;
		default:
			ret_flag = false;		
			break;
		}
	}
	
	return ret_flag ;
}

/*
static uint8_t convert_single_tbr_msg_into_uint(char *single_msg, uint8_t *dst_buf, uint8_t offset, uint32_t *first_timestamp) {
	char			*temp_ptr;
	char			ref_token[2] = ",";
	char			*token;
	bool			message_type = TBR_DETECION_MSG;
	tbr_message_t	tbr_message;
	tbr_message.CodeData = 0;  	//TODO Check that this does no harm
	tbr_message.CodeID = 0;		//TODO Check that this does no harm
	tbr_message.CodeType = 0;	//TODO Check that this does no harm
	int				diff_freq = 0;
	uint8_t			detection_freq = 0;
	uint8_t			code_type_unsigned = 0;
	uint8_t			buf_index = 0;
	uint8_t			protocol  = 0;
	bool			diff_flag = true;

    //$000xxx
	token = strtok(single_msg, ref_token);  //originally strtok_r
    //timestamp
	token = strtok(NULL, ref_token);  //originally strtok_r
	if (*first_timestamp > 0) {
		// NB! Does not handle *potential* rare case of overflow: (-1 = 255) 
		uint8_t timeDiff = (uint8_t)((uint32_t)strtoul(token,&temp_ptr,10) - *first_timestamp);
		// if(timeDiff==255){timeDiff=0;}  // handles overflow
		tbr_message.timeDiff = (timeDiff);
	}
	else {
		*first_timestamp = (uint32_t)strtoul(token,&temp_ptr,10);
		diff_flag = false;
	}

	//TBR Sensor or millisec
	token = strtok(NULL, ref_token);  //originally strtok_r
	if (strncmp(token, (char *)"TBR Sensor", 5) == 0) message_type = TBR_SENSOR_MSG;
	else tbr_message.millisec=(uint16_t)strtoul(token, &temp_ptr, 10);

	//Codetype or Temperature
	token = strtok(NULL, ref_token);  //originally strtok_r
	if(message_type == TBR_DETECION_MSG){
		if((strncmp(token,(char *)"R256", 4) == 0) || (strncmp(token, (char *)"r256",4) == 0)){
			tbr_message.CodeType = 00;
		}
		else if((strncmp(token, (char *)"R04K", 4) == 0) || (strncmp(token, (char *)"r04k", 4) == 0 ) ||
				(strncmp(token, (char *)"R04k", 4) == 0) || (strncmp(token, (char *)"r04K", 4) == 0 )) {
			tbr_message.CodeType = 01;
		}
		else if((strncmp(token, (char *)"R64K", 4) == 0) || (strncmp(token, (char *)"r64k", 4) == 0) ||
				(strncmp(token, (char *)"R64k", 4) == 0) || (strncmp(token, (char *)"r64K", 4) == 0)) {
			tbr_message.CodeType = 02;
		}
		else if((strncmp(token,(char *)"S256", 4) == 0) || (strncmp(token, (char *)"s256", 4) == 0)) {
			tbr_message.CodeType = 03;
		}
		else if((strncmp(token,(char *)"R01M", 4) == 0) || (strncmp(token, (char *)"r01m", 4)==0) ||
				(strncmp(token,(char *)"R01m", 4) == 0) || (strncmp(token, (char *)"r01M", 4)==0)) {
			tbr_message.CodeType = 04;
		}
		else if((strncmp(token,(char *)"S64K", 4) == 0) || (strncmp(token, (char *)"s64k", 4) == 0) ||
				(strncmp(token,(char *)"S64k", 4) == 0) || (strncmp(token, (char *)"s64K", 4) == 0)) {
			tbr_message.CodeType = 05;
		}
		else if((strncmp(token,(char *)"HS256", 4) == 0) || (strncmp(token, (char *)"hs256", 4) == 0) ||
				(strncmp(token,(char *)"Hs256", 4) == 0) || (strncmp(token, (char *)"hS256", 4) == 0)) {
			tbr_message.CodeType = 06;
		}
		else if((strncmp(token,(char *)"DS256", 4) == 0) || (strncmp(token, (char *)"ds256", 4) == 0) ||
				(strncmp(token,(char *)"Ds256", 4) == 0) || (strncmp(token, (char *)"dS256", 4) == 0)) {
			tbr_message.CodeType = 07;
		}
		else tbr_message.CodeType = 0xFE;

		protocol = tbr_message.CodeType;
	}
	else tbr_message.Temperature = (uint16_t)strtoul(token, &temp_ptr, 10);

    //CodeID or Noise
	token = strtok(NULL, ref_token);  //originally strtok_r(NULL, ref_token, &save)
	if(message_type == TBR_DETECION_MSG) tbr_message.CodeID = (uint32_t)strtoul(token, &temp_ptr, 10);
	else tbr_message.Noise = (uint8_t)strtoul(token, &temp_ptr, 10);

    //CodeData or NoiseLP
	token = strtok(NULL, ref_token);  //originally strtok_r
	if(message_type == TBR_DETECION_MSG) tbr_message.CodeData = (uint16_t)strtoul(token, &temp_ptr, 10);
	else tbr_message.NoiseLP=(uint8_t)strtoul(token,&temp_ptr,10);

    //SNR or Frequency
	token = strtok(NULL, ref_token);  //originally strtok_r
	if(message_type == TBR_DETECION_MSG) tbr_message.SNR=(uint8_t)strtoul(token, &temp_ptr, 10);
	else tbr_message.frequency=(uint8_t)strtoul(token, &temp_ptr, 10);

	//change code type as per frequency
	token = strtok(NULL, ref_token);  //originally strtok_r
	if (message_type == TBR_DETECION_MSG) {
		detection_freq = (uint8_t)strtoul(token, &temp_ptr, 10);
		diff_freq = detection_freq-69;
		if (detection_freq > 69) {
			code_type_unsigned = tbr_message.CodeType+(16*diff_freq);
			tbr_message.CodeType = code_type_unsigned;
		}
		else if (detection_freq<69) {
			code_type_unsigned = (uint8_t)(tbr_message.CodeType+(16*(diff_freq-1)));
			tbr_message.CodeType = code_type_unsigned;
		}
		else {		//in case of 69 KHz do not change code type
			;
		}
	}

    //fill the lora buffer
	if (diff_flag) dst_buf[offset+0] = (uint8_t)tbr_message.timeDiff;
	else {
		dst_buf[offset+0] = (uint8_t)(*first_timestamp>>24);
		dst_buf[offset+1] = (uint8_t)(*first_timestamp>>16);
		dst_buf[offset+2] = (uint8_t)(*first_timestamp>>8);
		dst_buf[offset+3] = (uint8_t)(*first_timestamp>>0);
		dst_buf[offset+4] = 0x00;
		buf_index = 4;
	}

	if(message_type == TBR_DETECION_MSG){
		dst_buf[offset+buf_index+1] = (uint8_t)tbr_message.CodeType;
		buf_index += 1;
		if(protocol == 0) {  //R256
			dst_buf[offset+buf_index+1] = (uint8_t)(tbr_message.CodeID);
			buf_index += 1;
		}
		else if((protocol == 1) || (protocol == 2)) {  //R04K || R64K
			dst_buf[offset+buf_index+1] = (uint8_t)(tbr_message.CodeID>>8);
			dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.CodeID>>0);
			buf_index += 2;
		}
		else if(protocol == 3) {  //S256
			dst_buf[offset+buf_index+1] = (uint8_t)tbr_message.CodeID;
			dst_buf[offset+buf_index+2] = (uint8_t)tbr_message.CodeData;
			buf_index += 2;
		}
		else if(protocol == 4) {  //R01M
			dst_buf[offset+buf_index+1] = (uint8_t)(tbr_message.CodeID>>16);
			dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.CodeID>>8);
			dst_buf[offset+buf_index+3] = (uint8_t)(tbr_message.CodeID>>0);
			buf_index += 3;
		}
		else if(protocol == 5) {  //S64K
			dst_buf[offset+buf_index+1] = (uint8_t)(tbr_message.CodeID>>8);
			dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.CodeID>>0);
			dst_buf[offset+buf_index+3] = (uint8_t)tbr_message.CodeData;
			buf_index += 3;
		}
		else if((protocol == 6) || (protocol == 7)) {  //HS256 || DS256
			dst_buf[offset+buf_index+1] = (uint8_t)tbr_message.CodeID;
			dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.CodeData>>8);
			dst_buf[offset+buf_index+3] = (uint8_t)(tbr_message.CodeData>>0);
			buf_index += 3;
		}
		else {  //should not happen
			;  // Do nothing, packet will be discarded at receiving end 
		}
		uint8_t temp_1 = ((tbr_message.SNR & 0x3F)<<2);
		uint8_t temp_2 = ((tbr_message.millisec>>8));
		dst_buf[offset+buf_index+1] = temp_1 | temp_2;
		dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.millisec>>0);
		buf_index += 2;
	}
	else {
		dst_buf[offset+buf_index+1] = (uint8_t)0xFF;
		dst_buf[offset+buf_index+2] = (uint8_t)(tbr_message.Temperature>>8);
		dst_buf[offset+buf_index+3] = (uint8_t)(tbr_message.Temperature>>0);
		dst_buf[offset+buf_index+4] = (uint8_t)tbr_message.Noise;
		dst_buf[offset+buf_index+5] = (uint8_t)tbr_message.NoiseLP;
		dst_buf[offset+buf_index+6] = (uint8_t)tbr_message.frequency;
		dst_buf[offset+buf_index+7] = 0xCC;                                        // Reserved for upper accuracy limit
		buf_index += 7;
	}
	return offset+buf_index+1;		//fixed offset=message size - 1.....
}
*/