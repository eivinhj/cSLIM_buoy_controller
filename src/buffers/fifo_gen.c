#include "fifo_gen.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(fifo_gen, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

void fifo_gen_init(fifo_gen_buffer_t* buf) {
	buf->size  = 0;
	buf->front = 0;
	buf->rear  = 0;
}


bool fifo_gen_is_empty(fifo_gen_buffer_t* buf) {
	bool temp_flag = false;

	if (buf->size == 0) temp_flag = true;
	return temp_flag;
}


bool fifo_gen_is_full(fifo_gen_buffer_t* buf) {
	bool temp_flag = false;
	
	if (buf->size == FIFO_GEN_MAX_SIZE) temp_flag = true;
	return temp_flag;
}


char fifo_gen_remove(fifo_gen_buffer_t* buf) {
	char temp_data = 0;

	if (buf->size>0){
		temp_data=buf->buffer[buf->front];
		buf->front++;
		buf->size--;
		if(buf->front == FIFO_GEN_MAX_SIZE) buf->front = 0;
	}
	return temp_data;
}


void fifo_gen_add(fifo_gen_buffer_t* buf, char data) {
	
	if (buf->size < FIFO_GEN_MAX_SIZE) {
		buf->buffer[buf->rear]=data;
		buf->rear++;
		buf->size++;
		if(buf->rear==FIFO_GEN_MAX_SIZE) buf->rear = 0;
	}
}

bool fifo_gen_contain_complete_string(fifo_gen_buffer_t* buf)
{
	//Chech for string that ends with "\r\n" or "\n\r"
	if(buf->size<2) return false; 
	uint8_t last;
	uint8_t second_last;
	if(buf->rear > 0) last = buf->buffer[buf->rear - 1];
	else last = buf->buffer[FIFO_GEN_MAX_SIZE - 1];

	if(buf->rear > 1) second_last= buf->buffer[buf->rear - 2];
	else second_last= buf->buffer[FIFO_GEN_MAX_SIZE - 2];

	//If charstring
	if(last == '\0')
	{
		//LOG_INF("contain \\0");
		return true;
	} 

	if ((last == '\r' && second_last == '\n') || (last == '\n' && second_last == '\r'))
	{
		//LOG_INF("contain \\r\\n");
		return true;
	}
		
	return false;
}

bool fifo_gen_contain_start_sign(fifo_gen_buffer_t* buf, char start_sign)
{
	if(buf->size<2) return false; 

	uint8_t first_index = buf->front;

	for(int i = 0; i < buf->size; i++)
	{
		if( buf->buffer[((first_index+i) % FIFO_GEN_MAX_SIZE)] == start_sign)
		{
			return true;			
		}
	}
		
	return false;
}

uint8_t fifo_gen_get_length(fifo_gen_buffer_t* buf)
{
	return buf->size;
}