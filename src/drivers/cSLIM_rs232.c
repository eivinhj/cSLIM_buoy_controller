/*
 * rs232.c
 *
 *  Created on: Mar 27, 2017
 *      Author: Waseemh
 */

#include "cSLIM_rs232.h"

 #include <zephyr.h>                                                                                                     
 #include <logging/log.h> 
 LOG_MODULE_REGISTER(cslim_rs232, CONFIG_MQTT_SIMPLE_LOG_LEVEL);                                                                                          
 #include <drivers/uart.h>  
 #include <drivers/gpio.h>

/*
 * Zephyr devices
 */
 #define UART1_DEVICE_NAME		DT_LABEL(DT_ALIAS(uart2))
 #define RS232_nSHDN_NODE	DT_ALIAS(rs232nshdn)

#if DT_NODE_HAS_STATUS(RS232_nSHDN_NODE, okay) && DT_NODE_HAS_PROP(RS232_nSHDN_NODE, gpios)
#define RS232_nSHDN_GPIO_LABEL	DT_GPIO_LABEL(RS232_nSHDN_NODE, gpios)
#define RS232_nSHDN_GPIO_PIN	DT_GPIO_PIN(RS232_nSHDN_NODE, gpios)
#define RS232_nSHDN_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(RS232_nSHDN_NODE, gpios))
#endif

 const struct device *uart_rs232;
 const struct device *gpio_rs232_nshdn;
 
 /* End of Zephyr devices*/


const unsigned char  	        rs232_tx_buf_gnss[256];
static 			char 			rs232_isr_rx_tx_char='0';

static void RS232_ISR(const struct device *dev, void* data);

//Note: nRF9160 has only four peripheral devices, so rs845, rs232, SPI, I2C, Uart to WLR and Wuart to logging can not be enablet simultaneously
int rs232_init( void ) {
    gpio_rs232_nshdn = device_get_binding(RS232_nSHDN_GPIO_LABEL);
    if (gpio_rs232_nshdn == NULL) 
    {
    printk("Didn't find GPIO device %s\n", RS232_nSHDN_GPIO_LABEL);
    return 0;
    }
    int ret = gpio_pin_configure(gpio_rs232_nshdn, RS232_nSHDN_GPIO_PIN, RS232_nSHDN_GPIO_FLAGS);
    if (ret != 0) 
    {
        printk("Error %d: failed to configure LED device %s pin %d\n",
                ret, RS232_nSHDN_GPIO_LABEL, RS232_nSHDN_GPIO_PIN);
        return ret;
    }
    gpio_pin_set(gpio_rs232_nshdn, RS232_nSHDN_GPIO_PIN, 1);
   
    if (ENABLE_RS232_FLAG) {
        uart_rs232 = device_get_binding(UART1_DEVICE_NAME);        
        if(uart_rs232 == NULL)
        {
            LOG_ERR("Uart RS232 not configured");
        }

        uart_irq_callback_user_data_set(uart_rs232, RS232_ISR, NULL); //TODO
        
        uart_irq_rx_enable(uart_rs232);
        uart_irq_tx_complete(uart_rs232);
        uart_irq_tx_enable(uart_rs232);

        uart_rx_enable(uart_rs232, &rs232_isr_rx_tx_char, 1, 1); 
        //fifo_rs232_init();
    }
    else 
    {
        rs232_shutdown();
    }
    return 0;
}

void rs232_enable( void ) {
    //if (ENABLE_RS232_FLAG) USART_Enable(RS232_USART, usartEnable);
}


void rs232_disable( void ) {
    //if (ENABLE_RS232_FLAG) USART_Enable(RS232_USART, usartDisable);
}


int rs232_transmit_string(const unsigned char* data,uint8_t length) {
    if (ENABLE_RS232_FLAG) {
        int loop_var = 0;
        int ret_val = 0;
        if (!fifo_rs232_is_full(fifo_tx_data)) {
            for (loop_var = 0; loop_var < length; loop_var++) {
                fifo_rs232_add(fifo_tx_data, data[loop_var]);
            }
            uart_irq_tx_enable(uart_rs232);
            ret_val = 1;
        } else {
            ret_val = -1;
        }
        return ret_val;
    }
    else return 1;
}


int rs232_transmit_char(uint8_t data) {
    if (ENABLE_RS232_FLAG) {
        int ret_val = 0;
        if (!fifo_rs232_is_full(fifo_tx_data)) {
            fifo_rs232_add(fifo_tx_data, data);
                    uart_irq_tx_enable(uart_rs232);
            ret_val = 1;
        } else {
            ret_val = -1;
        }
        return ret_val;
    }
    else return 1;
}


char rs232_receive( void ) {
	char temp_char='@';
    if (ENABLE_RS232_FLAG) {
        if (!fifo_rs232_is_empty(fifo_rx_data)) {
            temp_char = fifo_rs232_remove(fifo_rx_data);
        }
    }
	return temp_char;
}


void rs232_reset( void ) {
    //if (ENABLE_RS232_FLAG) USART_Reset(RS232_USART);
}


void rs232_shutdown( void ) {
	gpio_pin_set(gpio_rs232_nshdn, RS232_nSHDN_GPIO_PIN, 0);
}


/*
 * INT handlers
 */

static void RS232_ISR(const struct device *dev, void* data) {
	uart_irq_update(uart_rs232);

	if (uart_irq_tx_ready(uart_rs232)){
        if (ENABLE_RS232_FLAG) {
            uart_irq_tx_complete(uart_rs232);
            if (!fifo_rs232_is_empty(fifo_tx_data)) {
                uint8_t to_send = fifo_rs232_remove(fifo_tx_data);
                uart_tx(uart_rs232, &to_send, 1, 1);
            } else {
            uart_irq_tx_disable(uart_rs232);
            }
            
        }
	}
	 if (uart_irq_rx_ready(uart_rs232)){
        if (ENABLE_RS232_FLAG) {
            
            if (!fifo_rs232_is_full(fifo_rx_data)) {
                fifo_rs232_add(fifo_rx_data, uart_rx_enable(uart_rs232, &rs232_isr_rx_tx_char, 1, 1));
            }
            //uart_irq_rx_complete(uart_rs232);
        }
	 }
	 //Clear interrupt?
}
