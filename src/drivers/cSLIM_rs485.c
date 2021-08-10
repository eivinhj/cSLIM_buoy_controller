/*
 * cSLIM rs485.c
 *
 *  Based on rs485.c: Mar 29, 2017
 *      Author: Waseemh
 * 
 *  Adapted for nRF on: Apr 2021
 *      Author: Eivinhj
 */

 #include <zephyr.h>                                                                                                     
 #include <logging/log.h> 
 LOG_MODULE_REGISTER(cslim_rs485, LOG_LEVEL_INF);                                                                                          
 #include <drivers/uart.h>  
 #include <drivers/gpio.h>

 #include "cSLIM_rs485.h"
 #include "../buffers/fifo_gen.h"

#include "../tasks/tbr_detect_task.h" //used by ISR
#include "../tasks/tbr_sync_task.h" //used by ISR
#include "../time/time_local.h"

////////////////////////////////////
// Zephyr devices
////////////////////////////////////
#define UART1_DEVICE_NAME		DT_LABEL(DT_ALIAS(uart1))
#define RS485_RE_NODE	DT_ALIAS(rs485re) 
#define RS485_DE_NODE	DT_ALIAS(rs485de)

//TODO ADD GPIO PINS 
//TODO ADD Interrupt callbacks 

#if DT_NODE_HAS_STATUS(RS485_RE_NODE, okay) && DT_NODE_HAS_PROP(RS485_RE_NODE, gpios)
#define RS485_RE_GPIO_LABEL	DT_GPIO_LABEL(RS485_RE_NODE, gpios)
#define RS485_RE_GPIO_PIN	DT_GPIO_PIN(RS485_RE_NODE, gpios)
#define RS485_RE_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(RS485_RE_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(RS485_DE_NODE, okay) && DT_NODE_HAS_PROP(RS485_DE_NODE, gpios)
#define RS485_DE_GPIO_LABEL	DT_GPIO_LABEL(RS485_DE_NODE, gpios)
#define RS485_DE_GPIO_PIN	DT_GPIO_PIN(RS485_DE_NODE, gpios)
#define RS485_DE_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(RS485_DE_NODE, gpios))
#endif

const struct device *gpio_rs485_re;
const struct device *gpio_rs485_de;
const struct device *uart_rs485;

const struct uart_config rs485_cfg = {
	.baudrate 	= 115200,
	.parity		= UART_CFG_PARITY_NONE,
	.stop_bits	= UART_CFG_STOP_BITS_1,
	.data_bits	= UART_CFG_DATA_BITS_8,
	.flow_ctrl	= UART_CFG_FLOW_CTRL_NONE,
};




////////////////////////////////////
// Private variables
////////////////////////////////////
//static			fifo_tbr_type_t			current_rx_fifo=fifo_tbr_rx_data;
static 			char 					isr_rx_tx_char='0';
static fifo_gen_buffer_t rx_buf;
static fifo_gen_buffer_t tx_buf;

bool sync_tx_with_10sec;
tbr_tx_char tx_char_status;


////////////////////////////////////
// Private function declarations
////////////////////////////////////
static void RS485_ISR(const struct device *dev, void*);

////////////////////////////////////
// Public functions
///////////////////////////////////

int rs485_init(void)
 {

	fifo_gen_init(&tx_buf);
	fifo_gen_init(&rx_buf);

	fifo_tbr_init();

	LOG_DBG("RS485 init");

	sync_tx_with_10sec = false;
	tx_char_status=NORMAL;
	int ret = 0;
	uart_rs485 = device_get_binding(UART1_DEVICE_NAME);        
	if(uart_rs485 == NULL)
	{
		LOG_ERR("Uart RS485 not configured");
	}
	uart_configure(uart_rs485, &rs485_cfg);
	LOG_DBG("RS485 uart configured");

	//Configure RE and DE pins
	gpio_rs485_re = device_get_binding(RS485_RE_GPIO_LABEL);
	if (gpio_rs485_re == NULL) {
		printk("Didn't find GPIO device %s\n", RS485_RE_GPIO_LABEL);
		return 0;
	}
	ret = gpio_pin_configure(gpio_rs485_re, RS485_RE_GPIO_PIN, RS485_RE_GPIO_FLAGS);
	if (ret != 0) {
		printk("Error %d: failed to configure GPIO device %s pin %d\n",
			ret, RS485_RE_GPIO_LABEL, RS485_RE_GPIO_PIN);
		return ret;
	}
	gpio_rs485_de = device_get_binding(RS485_DE_GPIO_LABEL);
	if (gpio_rs485_de == NULL) 
	{
		printk("Didn't find GPIO device %s\n", RS485_DE_GPIO_LABEL);
		return 0;
	}
	ret = gpio_pin_configure(gpio_rs485_de, RS485_DE_GPIO_PIN, RS485_DE_GPIO_FLAGS);
	if (ret != 0) 
	{
		printk("Error %d: failed to configure GPIO device %s pin %d\n",
			ret, RS485_DE_GPIO_LABEL, RS485_DE_GPIO_PIN);
		return ret;
	}
	gpio_pin_set(gpio_rs485_re, RS485_RE_GPIO_PIN, 1);
	gpio_pin_set(gpio_rs485_de, RS485_DE_GPIO_PIN, 1);


 	uart_irq_update(uart_rs485);
	uart_rx_enable(uart_rs485, rx_buf.buffer, FIFO_GEN_MAX_SIZE, 1);
	uart_irq_callback_user_data_set(uart_rs485, RS485_ISR, NULL); 
	uart_irq_rx_enable(uart_rs485);
	uart_irq_tx_complete(uart_rs485);
	uart_irq_tx_enable(uart_rs485);

	rs485_rx_mode();
	uart_rx_enable(uart_rs485, &isr_rx_tx_char, 1, 1);
	return 0; 
 }

void rs485_enable(void) {
	uart_rx_enable(uart_rs485, &isr_rx_tx_char, 1, 1);
	rs485_rx_mode();
}


void rs485_disable(void) {
	uart_rx_disable(uart_rs485);
	rs485_low_power_mode(true);
}

void rs485_reset(void) {
	rs485_flush_rx_buffer();
}

int rs485_transmit_string(const unsigned char* data, uint8_t length, bool sync_local_time) {
		rs485_tx_mode();
		k_sleep(K_MSEC(1));
        int loop_var = 0;
        int ret_val = 0;
        if (!fifo_gen_is_full(&tx_buf)) {
            for (loop_var = 0; loop_var < length; loop_var++) {
				if(loop_var == length-1 && sync_local_time)
				{
					tx_char_status = SET;
				}
                fifo_gen_add(&tx_buf, data[loop_var]);
				uart_irq_tx_enable(uart_rs485);
				k_sleep(K_MSEC(1));

            }
            
            ret_val = 1;
        } else {
            ret_val = -1;
        }
		k_sleep(K_MSEC(1));
		rs485_rx_mode();
        return ret_val;

}

int rs485_transmit_char(uint8_t data) {
		rs485_tx_mode();
		k_sleep(K_MSEC(1));

        int ret_val = 0;
        if (!fifo_gen_is_full(&tx_buf)) {
            fifo_gen_add(&tx_buf, data);
                    uart_irq_tx_enable(uart_rs485);
					k_sleep(K_MSEC(1));
            ret_val = 1;
        } else {
            ret_val = -1;
        }
		k_sleep(K_MSEC(1));
		rs485_rx_mode();
        return ret_val;
}

bool rs485_receive_char(uint8_t* c){
	*c ='@';

        if (!fifo_gen_is_empty(&rx_buf)) {
            (*c) = fifo_gen_remove(&rx_buf);
            return true;
        }
	return false;
}


void rs485_tx_mode(void) {
	uart_rx_disable(uart_rs485);
	gpio_pin_set(gpio_rs485_re, RS485_RE_GPIO_PIN, 1);
	gpio_pin_set(gpio_rs485_de, RS485_DE_GPIO_PIN, 1);
}


void rs485_rx_mode(void) {
	gpio_pin_set(gpio_rs485_re, RS485_RE_GPIO_PIN, 0);
	gpio_pin_set(gpio_rs485_de, RS485_DE_GPIO_PIN, 0);
	k_sleep(K_MSEC(1));
	uart_rx_enable(uart_rs485, &isr_rx_tx_char, 1, 1);
}


void rs485_low_power_mode( bool flag ) {
    if ( flag ) {
		gpio_pin_set(gpio_rs485_re, RS485_RE_GPIO_PIN, 1);
		gpio_pin_set(gpio_rs485_de, RS485_DE_GPIO_PIN, 0);
    }
    else rs485_rx_mode();


}

void rs485_flush_rx_buffer()
{
	char temp_char = '0';
	while(temp_char != '@') {
		rs485_receive_char(&temp_char);
	}
}

////////////////////////////////////
// Private functions
////////////////////////////////////




/*
 * INT handlers
 */
static void RS485_ISR(const struct device *dev, void* data) {
	//LOG_DBG("IRQ callback");

	/////////////////////////
	uart_irq_update(dev);
	uint32_t k_cycle = k_cycle_get_32();

	if (uart_irq_tx_ready(dev)){
        //LOG_INF("ISR TX Ready");
        
            if (!fifo_gen_is_empty(&tx_buf)) {
				//k_sleep(K_MSEC(1)); //TODO find way this does not have to sleep in ISR
                uint8_t to_send = fifo_gen_remove(&tx_buf);
                uint8_t bytes = uart_fifo_fill(dev, &to_send, 1);
				
				(void) bytes;
            } else {
            uart_irq_tx_disable(dev);
            }

			if(tx_char_status == SET)
			{
				tx_char_status = BEING_HANDLED;//wait for tx complete
			}
			else if(tx_char_status == BEING_HANDLED)
			{
				tx_char_status = NORMAL;
				//send timestamp to correct TBR message
				struct tm time_tm_temp;
				uint32_t time_us;
				get_precise_local_date_and_time_tm(&time_tm_temp, &time_us, &k_cycle);
				uint8_t offset_sec = time_tm_temp.tm_sec % 10;
				if(offset_sec > 5) offset_sec -= 10;
				tbr_sync_set_offset(offset_sec, time_us);
				rs485_rx_mode();
			}
            
        
        uart_irq_tx_complete(dev);
	}

	if (uart_irq_rx_ready(dev)){
        //LOG_INF("ISR RX Ready");
       	uint8_t receive_char;
		uart_fifo_read(dev, &receive_char, 1);
		LOG_DBG("Receive %c", receive_char);
		//rs485_tx_mode();   

            if (!fifo_gen_is_full(&rx_buf)) {
                

                fifo_gen_add(&rx_buf, receive_char);
				
				if(receive_char == '\r')
				{
					//LOG_INF("Receive \r");
					 if(fifo_gen_contain_start_sign(&rx_buf, '$'))
					{
						LOG_INF("New TBR detection");
						
						tbr_detect_task_msgq_data_item_t tbr_detect_task_msgq_data_item;
						tbr_detect_task_msgq_data_item.message_length = fifo_gen_get_length(&rx_buf);
						LOG_DBG("Length: %d", tbr_detect_task_msgq_data_item.message_length);
						for(int i = 0; i < tbr_detect_task_msgq_data_item.message_length; i++)
						{	
							tbr_detect_task_msgq_data_item.message[i] = fifo_gen_remove(&rx_buf);
						}
						uint8_t offset_seconds;
						uint32_t offset_usec;
						tbr_sync_get_offset(&offset_seconds, &offset_usec);
						tbr_detect_task_msgq_data_item.tbr_offset_sec=offset_seconds;
						tbr_detect_task_msgq_data_item.tbr_offset_usec=offset_usec;

						tbr_detect_task_add_msg(&tbr_detect_task_msgq_data_item);
						
					}
				}
				
            }


		/*for(int i = 0; i < 1000; i++)
		{
			uart_fifo_read(dev, &receive_char, 1);
		}*/

	 }
	
}
/*void change_rx_fifo_type(fifo_tbr_type_t rx_fifo_type){
	current_rx_fifo=rx_fifo_type;
	return;
}*/

