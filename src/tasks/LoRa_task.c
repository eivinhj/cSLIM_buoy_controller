

//TODOs
//Get datarate from LoRa module and match the number of bytes sent with the corresponding datarate. Max is 6 tag detections for DR1 and 33 for DR5, inbetween not known.
// For now a counter is making sure only 5 messages are sent at a time
//check that messages are transmitted correctly by receiving mac_tx_ok
//LoRa module does not work well with logging on the same uart.

#include "LoRa_task.h"
#include "display_task.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include "../devices/display.h"

#include "../devices/wlr089u0.h"
#include "../buffers/fifo_gen.h"

#include "../mqtt/cSLIM_MQTT.h"

LOG_MODULE_REGISTER(LoRa_task, LOG_LEVEL_DBG);

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 128

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

K_MSGQ_DEFINE(lora_tbr_tag_detections, sizeof(tbr_tag_detection_t), 20, 32);
K_MSGQ_DEFINE(lora_tbr_log_messages, sizeof(tbr_log_t), 20, 32);
K_MSGQ_DEFINE(lora_cslim_status_messages, sizeof(cslim_status_t), 10, 32);

#define LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH 200 //match with size of lora_tbr_tag_detections and tbr_log_ above, much fit messages plus headers (max 33 tag detections for dr5, max 6 for dr1)
#define MAX_BYTES_PER_UART_TX 30

K_MSGQ_DEFINE(loRa_task_msgq, sizeof(gps_task_msgq_data_item_t), 1, 128);

const unsigned char inf_str[] = "sys get ver\r\n";
const unsigned char reset_868[] = "mac reset 868\r\n";
const unsigned char set_adr_on[] = "mac set adr on\r\n";
const unsigned char dev_eui[] = "mac set deveui 8278000000000600\r\n";
const unsigned char join_eui[] = "mac set joineui 0011224455119988\r\n";
const unsigned char app_key[] = "mac set appkey 00110022003300440055006600770088\r\n";
const unsigned char pwridx[] = "mac set pwridx 0\r\n";
const unsigned char join_otaa[] = "mac join otaa\r\n";
const unsigned char set_datarate[] = "mac set dr 4\r\n";
const unsigned char sleep_60s[] = "sys sleep standby 60000\r\n";
const unsigned char lora_tx_cnf[] = "mac tx cnf 1 ";

//char err_invalid_param[] = "invalid_param";
char wlr_reply_success[] = "USER";
char wlr_reply_err[] = "err";
char wlr_reply_accepted[] = "accepted";
char wlr_reply_invalid_param[] = "invalid_param";
char wlr_reply_ok[] = "ok";
char wlr_reply_busy[] = "busy";
char wlr_reply_tx_ok[] = "mac_tx_ok";


int lora_init_complete = 0; 

gps_task_msgq_data_item_t  loRa_task_msgq_item;

/////////////////////////////////////////////////
/// Private function declarations
/////////////////////////////////////////////////
void lora_configure();
int wlr_send_cmd(const unsigned char* str, uint8_t strlen);
int lora_join_otaa();
int lora_transmit_ack(const unsigned char* str, uint8_t strlen);
static void WLR089U0_ISR(const struct device *dev, void* data);
int add_lora_tx_message_to_buffer(uint8_t* buffer, uint16_t* index);
int lora_task_send_tag_detections();
int lora_task_send_tbr_log();
int lora_task_send_cslim_status();


/////////////////////////////////////////////////
/// Public functions
/////////////////////////////////////////////////

void lora_task_add_tag_detection(tbr_tag_detection_t* tag_msg)
{
	int err = k_msgq_put(&lora_tbr_tag_detections, tag_msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(lora_init_complete)
		{
			lora_task_send_tag_detections();
			k_msgq_put(&lora_tbr_tag_detections, tag_msg, K_NO_WAIT);	
		}
		
	}
}

void lora_task_add_tbr_log_message(tbr_log_t* log_msg)
{
	int err = k_msgq_put(&lora_tbr_log_messages, log_msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(lora_init_complete)
		{
			lora_task_send_tbr_log();
			k_msgq_put(&lora_tbr_log_messages, log_msg, K_NO_WAIT);
		}
		
	}
}

void lora_task_add_cslim_status_message(cslim_status_t* msg)
{
	int err = k_msgq_put(&lora_cslim_status_messages, msg, K_NO_WAIT);
	if(err == -EAGAIN) //msgq is full
	{
		if(lora_init_complete)
		{
			lora_task_send_cslim_status();
			k_msgq_put(&lora_cslim_status_messages, msg, K_NO_WAIT);
		}
		
	}
}


void loRa_task(void)
{
	LOG_INF("Is thread ID %x", (int) k_current_get());
	k_sleep(K_SECONDS(5));
	display_put_string(6, 1 * 12 + 4, "LoRa: Device? ", font_medium);
	request_display_update();
	gps_task_msgq_data_item_t  loRa_task_msgq_item;
	wlr089u0_init(WLR089U0_ISR);

	  //This is using same uart as RS485 and has hence been disabled. TODO: Find a way to mux the uarts or rewrite the WLR code to use i2c or spi. 
	//clear FIFO queue
	LOG_INF("WLR:Clear messages");

	k_msgq_purge(&loRa_task_msgq);
	
	k_sleep(K_SECONDS(6));
	lora_configure();

	

	while(1)
	{
		if(k_msgq_get(&loRa_task_msgq, &loRa_task_msgq_item, K_MSEC(1000)) == 0)
		{
			
			if(!strncmp(wlr_reply_err, loRa_task_msgq_item.message, 3))
			{
				LOG_ERR("Receive err");
			}
			else if(loRa_task_msgq_item.message_length < 4)
			{
				LOG_INF("Message too short");
			}
			else if(!strncmp(wlr_reply_ok, loRa_task_msgq_item.message, 2))
			{
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));

			}
			else if(!strncmp(wlr_reply_accepted, loRa_task_msgq_item.message, 8))
			{
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
			}

			else
			{
				
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
			}
		}
		else
		{
			
			wlr089u0_transmit_string(sleep_60s, sizeof(sleep_60s)); // do not expect ack until lora re-wakes
			k_sleep(K_SECONDS(60));
			//TODO check for received messages if it should be supported
			k_msgq_purge(&loRa_task_msgq);
			
		}
		LOG_INF("LoRa sending messages if any ---");
		lora_task_send_tag_detections();
		lora_task_send_tbr_log();
		lora_task_send_cslim_status();

		
	}
}


/////////////////////////////////////////////////
// Local functions
/////////////////////////////////////////////////


void lora_configure()
{
//set parameters
	display_put_string(6, 1 * 12 + 4, "LoRa: Config ", font_medium);
	request_display_update();
	lora_init_complete=0;
	while(wlr_send_cmd(reset_868, sizeof(reset_868)));
	while(wlr_send_cmd(dev_eui, sizeof(dev_eui)));
	while(wlr_send_cmd(join_eui, sizeof(join_eui)));
	while(wlr_send_cmd(app_key, sizeof(app_key)));
	wlr_send_cmd(pwridx, sizeof(pwridx));
	wlr_send_cmd(set_adr_on, sizeof(set_adr_on));
	wlr_send_cmd(set_datarate, sizeof(set_datarate));
	display_put_string(6, 1 * 12 + 4, "LoRa:Search ", font_medium);
	request_display_update();
	lora_join_otaa();
	lora_init_complete=1;
	display_put_string(6, 1 * 12 + 4, "LoRa: Joined", font_medium);
	request_display_update();
}


int lora_task_send_tag_detections()
{
	char lora_buffer[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t lora_buffer_index = 0; 
	tbr_tag_detection_t tag_detection;

	//Check that MQTT client is ready
	if(!lora_init_complete)
	{
		LOG_ERR("LoRa init not complete");
		return -1;
	} 
	int status = k_msgq_get(&lora_tbr_tag_detections, &tag_detection, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		LOG_INF("No TBR Tag messages to be sent");
		return status;
	}

	//add_lora_tx_message_to_buffer(lora_buffer, &lora_buffer_index);

	// make IOF header
	time_t reference_timestamp = tag_detection.timestamp_unix;
	//LOG_INF("TBR SERIAL: %d", tag_detection.tbr_serial_number);
	//LOG_INF("REF TS: %d", reference_timestamp);
	tbr_compressed_tag_msg_make_header(tag_detection.tbr_serial_number, reference_timestamp, lora_buffer, &lora_buffer_index);
	//add tag detections
	int counter = 0;
	do {
		status = tbr_compressed_tag_msg_add_tag(&tag_detection, reference_timestamp, lora_buffer, &lora_buffer_index);
		counter++;
		if(status == -EINVAL || lora_buffer_index > LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH -10 || counter > 4)
		{
			break;
		}
	} while(k_msgq_get(&lora_tbr_tag_detections, &tag_detection, K_NO_WAIT) == 0);



	//Convert to hex string
	char converted[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH*2 + 1];
	char tx_buf[2*LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH+13+3];
	for(int i=0;i<lora_buffer_index;i++) {
    	sprintf(&converted[i*2], "%02X", lora_buffer[i]);
  	}
	strcpy(tx_buf, lora_tx_cnf);
	strcat(tx_buf, converted);
	lora_buffer_index = 13 + 2*lora_buffer_index;


	tx_buf[(lora_buffer_index)++] = '\r';
	tx_buf[(lora_buffer_index)++] = '\n';

	//send
	LOG_DBG("Send Tag detections LoRa");
	wlr_send_cmd(tx_buf, lora_buffer_index);

	if( k_msgq_peek(&lora_tbr_tag_detections, &tag_detection) == 0) //more messages in queue
	{
		int lora_task_send_tag_detections();
	}
	return 0; 
}

int lora_task_send_tbr_log()
{
	char lora_buffer[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t lora_buffer_index = 0; 
	tbr_log_t tbr_log;

	//Check that MQTT client is ready
	if(!lora_init_complete)
	{
		LOG_ERR("LoRa init not complete");
		return -1;
	} 

	int status = k_msgq_get(&lora_tbr_log_messages, &tbr_log, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		LOG_DBG("No tbr log message to send");
		return status;
	}

	//add_lora_tx_message_to_buffer(lora_buffer, &lora_buffer_index);

	// make IOF header
	time_t reference_timestamp = tbr_log.timestamp_unix;
	tbr_compressed_log_msg_make_header(tbr_log.tbr_serial_number, reference_timestamp, lora_buffer, &lora_buffer_index);
	//add tag detections
	do {
		status = tbr_compressed_log_msg_add_tag(&tbr_log, reference_timestamp, lora_buffer, &lora_buffer_index);
		if(status == -EINVAL || lora_buffer_index > LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH -10)
		{
			break;
		}
	} while(k_msgq_get(&lora_tbr_log_messages, &tbr_log, K_NO_WAIT) == 0);


	//Convert to hex string
	char converted[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH*2 + 1];
	char tx_buf[2*LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH+13+3];
	for(int i=0;i<lora_buffer_index;i++) {
    	sprintf(&converted[i*2], "%02X", lora_buffer[i]);
  	}
	strcpy(tx_buf, lora_tx_cnf);
	strcat(tx_buf, converted);
	lora_buffer_index = 13 + 2*lora_buffer_index;

	tx_buf[(lora_buffer_index)++] = '\r';
	tx_buf[(lora_buffer_index)++] = '\n';

	//send
	LOG_DBG("Send TBR Log LoRa?");
	wlr_send_cmd(tx_buf, lora_buffer_index);

	if( k_msgq_peek(&lora_tbr_log_messages, &tbr_log) == 0) //more messages in queue
	{
		lora_task_send_tbr_log();
	}
	return 0; 
}

int lora_task_send_cslim_status()
{
	
	char lora_buffer[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH];
	uint16_t lora_buffer_index = 0; 
	cslim_status_t msg;

	//Check that MQTT client is ready
	if(!lora_init_complete)
	{
		LOG_ERR("LoRa init not complete");
		return -1;
	} 

	int status = k_msgq_get(&lora_cslim_status_messages, &msg, K_NO_WAIT);
	
	//Check that there is at least one message to be sent
	if(status != 0)
	{
		LOG_ERR("No status message to send");
		return status;
	}


	//add_lora_tx_message_to_buffer(lora_buffer, &lora_buffer_index);
	// make IOF header
	time_t reference_timestamp = msg.timestamp_unix;
	cslim_status_msg_make_header(msg.tbr_serial_number, reference_timestamp, lora_buffer, &lora_buffer_index);
	//add tag detections
	do {
		status = cslim_status_msg_add_msg(&msg, lora_buffer, &lora_buffer_index);
		if(status == -EINVAL || lora_buffer_index > LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH -10)
		{
			break;
		}
	} while(k_msgq_get(&lora_cslim_status_messages, &msg, K_NO_WAIT) == 0);


	//convert to hex string
	
	//Convert to hex string
	char converted[LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH*2 + 1];
	char tx_buf[2*LORA_TBR_MESSAGE_BUFFER_MAX_LENGTH+13+3];
	for(int i=0;i<lora_buffer_index;i++) {
    	sprintf(&converted[i*2], "%02X", lora_buffer[i]);
  	}
	strcpy(tx_buf, lora_tx_cnf);
	strcat(tx_buf, converted);
	lora_buffer_index = 13 + 2*lora_buffer_index;


	tx_buf[(lora_buffer_index)++] = '\r';
	tx_buf[(lora_buffer_index)++] = '\n';

	//send
	LOG_DBG("Send cSLIM status LoRa");
	wlr_send_cmd(tx_buf, lora_buffer_index);

	if( k_msgq_peek(&lora_cslim_status_messages, &msg) == 0) //more messages in queue
	{
		int lora_task_send_cslim_status();
	}
	return 0; 
}



int wlr_send_cmd(const unsigned char* str, uint8_t strlen)
{
	int error_count = 0;
	while(1)
	{
		
		k_msgq_purge(&loRa_task_msgq);
		int bytes_left = strlen;
		while(bytes_left > 0)
		{
			if(bytes_left > MAX_BYTES_PER_UART_TX)
			{
				wlr089u0_transmit_string(str + strlen - bytes_left, MAX_BYTES_PER_UART_TX);
				bytes_left -= MAX_BYTES_PER_UART_TX;
			}
			else
			{
				wlr089u0_transmit_string(str + strlen - bytes_left, bytes_left);
				bytes_left = 0;
			}
			
		}
		
		
		if(k_msgq_get(&loRa_task_msgq, &loRa_task_msgq_item, K_MSEC(1000)) == 0)
		{
			
			if(!strncmp(wlr_reply_err, loRa_task_msgq_item.message, 3))
			{
				LOG_ERR("Receive err");
			}
			else if(loRa_task_msgq_item.message_length < 4)
			{
				LOG_INF("Message too short");
			}
			else if(!strncmp(wlr_reply_ok, loRa_task_msgq_item.message, 2))
			{
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
				break;
			}
			else if(!strncmp(wlr_reply_accepted, loRa_task_msgq_item.message, 8))
			{
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
				break;
			}

			else
			{
				
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
			}
		}
		else
		{
			LOG_ERR("WLR Read timeout");
		}
		error_count++;
		if (error_count > 10)
		{
			return -1;
		}
		k_sleep(K_MSEC(10000));
	}
	//LOG_DBG("Return from wlr_send_cmd");
	return 0;
}

int lora_join_otaa()
{
	while(1)
	{
		wlr_send_cmd(join_otaa, sizeof(join_otaa));
		
		if(k_msgq_get(&loRa_task_msgq, &loRa_task_msgq_item, K_MSEC(7000)) == 0)
		{
			
			if(!strncmp(wlr_reply_err, loRa_task_msgq_item.message, 3))
			{
				LOG_ERR("Receive err");
			}
			else if(loRa_task_msgq_item.message_length < 4)
			{
				LOG_INF("Message too short");
			}
			else if(!strncmp(wlr_reply_accepted, loRa_task_msgq_item.message, 8))
			{
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
				break;
			}

			else
			{
				
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
			}
		}
		k_sleep(K_MSEC(10000));

	}
	return 0; 
}

int lora_transmit_ack(const unsigned char* str, uint8_t strlen)
{
	int busy_count = 0;
	while(1)
	{
		wlr_send_cmd(str, strlen);
		
		if(k_msgq_get(&loRa_task_msgq, &loRa_task_msgq_item, K_MSEC(7000)) == 0)
		{
			
			if(!strncmp(wlr_reply_err, loRa_task_msgq_item.message, 3))
			{
				LOG_ERR("Receive err");
			}
			else if(loRa_task_msgq_item.message_length < 4)
			{
				LOG_INF("Message too short");
			}
			else if(!strncmp(wlr_reply_tx_ok, loRa_task_msgq_item.message, 8))
			{
				LOG_INF("Message sent successfully ");
				break;
			}
			else if(!strncmp(wlr_reply_busy, loRa_task_msgq_item.message, 4))
			{
				LOG_INF("busy");
				busy_count++;
				if(busy_count > 10)
				{
					lora_configure();
					lora_transmit_ack(str, strlen);
					break;
				}
			}

			else
			{
				
				LOG_INF("Received: %s", log_strdup(loRa_task_msgq_item.message));
			}
		}
		k_sleep(K_MSEC(10000));

	}
	return 0; 

}

int add_lora_tx_message_to_buffer(uint8_t* buffer, uint16_t* index)
{
	buffer[(*index)++] = 'm';
	buffer[(*index)++] = 'a';
	buffer[(*index)++] = 'c';
	buffer[(*index)++] = ' ';
	buffer[(*index)++] = 't';
	buffer[(*index)++] = 'x';
	buffer[(*index)++] = ' ';
	buffer[(*index)++] = 'c';
	buffer[(*index)++] = 'n';
	buffer[(*index)++] = 'f';
	buffer[(*index)++] = ' ';
	buffer[(*index)++] = '1';
	buffer[(*index)++] = ' ';
	return 0;
}

/*
 * INT handlers
 */

static void WLR089U0_ISR(const struct device *dev, void* data) {
	uart_irq_update(dev);

	if (uart_irq_tx_ready(dev)){
        //LOG_INF("ISR TX Ready");
        
            if (!fifo_gen_is_empty(get_tx_buffer())) {
                uint8_t to_send = fifo_gen_remove(get_tx_buffer());
                uint8_t bytes = uart_fifo_fill(dev, &to_send, 1);
				(void) bytes;
                /*if(bytes){
                    LOG_INF("%d Bytes sent", bytes);
                }
                else{
                    LOG_ERR("No bytes sent");
                }*/
            } else {
            uart_irq_tx_disable(dev);
            }
            
        
        uart_irq_tx_complete(dev);
	}

	if (uart_irq_rx_ready(dev)){
        //LOG_INF("ISR RX Ready");
       
            
            if (!fifo_gen_is_full(get_rx_buffer())) {
                uint8_t receive_char;
                uart_fifo_read(dev, &receive_char, 1);
                fifo_gen_add(get_rx_buffer(), receive_char);
				
                if(fifo_gen_contain_complete_string(get_rx_buffer()))
                {
					gps_task_msgq_data_item_t loRa_task_ISR_msgq_item;
					loRa_task_ISR_msgq_item.message_length = fifo_gen_get_length(get_rx_buffer());
					LOG_INF("Length: %d", loRa_task_ISR_msgq_item.message_length);
					for(int i = 0; i < loRa_task_ISR_msgq_item.message_length; i++)
					{
						loRa_task_ISR_msgq_item.message[i] = fifo_gen_remove(get_rx_buffer());
					}

					 while (k_msgq_put(&loRa_task_msgq, &loRa_task_ISR_msgq_item, K_NO_WAIT) != 0) {
            			/* message queue is full: purge old data & try again */
            			k_msgq_purge(&loRa_task_msgq);
        			}
                }
            }
        
	 }
	 //Clear interrupt?
}	

