/*
 * wlr089u0.c
 *
 *  Created on: Mar 27, 2017
 *      Author: Waseemh
 */

#include "wlr089u0.h"

 #include <zephyr.h>                                                                                                     
 #include <logging/log.h> 
 LOG_MODULE_REGISTER(cslim_wlr089u0, LOG_LEVEL_INF);
                                                                                            
 #include <drivers/gpio.h>
 #include <drivers/pinmux.h>
 #include "drivers/cSLIM_SPI.h"

/*
 * Zephyr devices
 */
 #define WLR_UART_DEVICE_NAME		DT_LABEL(DT_ALIAS(uart0))
 #define WLR089U0_nEN_NODE	DT_ALIAS(wlr089u0nen)

#if DT_NODE_HAS_STATUS(WLR089U0_nEN_NODE, okay) && DT_NODE_HAS_PROP(WLR089U0_nEN_NODE, gpios)
#define WLR089U0_nEN_GPIO_LABEL	DT_GPIO_LABEL(WLR089U0_nEN_NODE, gpios)
#define WLR089U0_nEN_GPIO_PIN	DT_GPIO_PIN(WLR089U0_nEN_NODE, gpios)
#define WLR089U0_nEN_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(WLR089U0_nEN_NODE, gpios))
#endif



 const struct device *uart_wlr089u0;
 const struct device *gpio_wlr089u0_nen;
 const struct device *gpio_wlr089u0_tx;
 
 /* End of Zephyr devices*/

static fifo_gen_buffer_t tx_buf;
static fifo_gen_buffer_t rx_buf;

uint8_t to_send;

int wlr089u0_init( uart_irq_callback_user_data_t uart_cb) {
	fifo_gen_init(&tx_buf);
	fifo_gen_init(&rx_buf);


    if (ENABLE_WLR089U0_FLAG) {
        cSLIM_CS_init(LORA);

        //Enable UART
        
        uart_wlr089u0 = device_get_binding(WLR_UART_DEVICE_NAME);        
        if(uart_wlr089u0 == NULL)
        {
            LOG_ERR("Uart WLR089U0 not configured");
        }
        


        uart_irq_update(uart_wlr089u0);
        uart_rx_enable(uart_wlr089u0, rx_buf.buffer, FIFO_GEN_MAX_SIZE, 1);
        

        //Enable pin
        gpio_wlr089u0_nen = device_get_binding(WLR089U0_nEN_GPIO_LABEL);
        if (gpio_wlr089u0_nen == NULL) 
        {
        printk("Didn't find GPIO device %s\n", WLR089U0_nEN_GPIO_LABEL);
        return 0;
        }
        int ret = gpio_pin_configure(gpio_wlr089u0_nen, WLR089U0_nEN_GPIO_PIN, WLR089U0_nEN_GPIO_FLAGS);
        if (ret != 0) 
        {
        printk("Error %d: failed to configure GPIO device %s pin %d\n",
                ret, WLR089U0_nEN_GPIO_LABEL, WLR089U0_nEN_GPIO_PIN);
        return ret;
        }
        gpio_pin_set(gpio_wlr089u0_nen, WLR089U0_nEN_GPIO_PIN, 0);
        

        k_sleep(K_SECONDS(4));
        

        uart_irq_callback_user_data_set(uart_wlr089u0, uart_cb, NULL); 
        uart_irq_rx_enable(uart_wlr089u0);
        uart_irq_tx_complete(uart_wlr089u0);
        uart_irq_tx_enable(uart_wlr089u0);


    }
    else wlr089u0_shutdown();
    return 0;
}

void wlr089u0_enable( void ) {
    
    if (ENABLE_WLR089U0_FLAG) gpio_pin_set(gpio_wlr089u0_nen, WLR089U0_nEN_GPIO_PIN, 0);
}


void wlr089u0_disable( void ) {
    if (ENABLE_WLR089U0_FLAG) gpio_pin_set(gpio_wlr089u0_nen, WLR089U0_nEN_GPIO_PIN, 1);
}


int wlr089u0_transmit_string(const unsigned char* data, uint8_t length) {
    if (ENABLE_WLR089U0_FLAG) {
        int loop_var = 0;
        int ret_val = 0;
        if (!fifo_gen_is_full(&tx_buf)) {
            for (loop_var = 0; loop_var < length; loop_var++) {
                fifo_gen_add(&tx_buf, data[loop_var]);
            }
            uart_irq_tx_enable(uart_wlr089u0);
            ret_val = 1;
        } else {
            ret_val = -1;
        }
        return ret_val;
    }
    else return 1;
}


int wlr089u0_transmit_char(uint8_t data) {
    if (ENABLE_WLR089U0_FLAG) {
        int ret_val = 0;
        if (!fifo_gen_is_full(&tx_buf)) {
            fifo_gen_add(&tx_buf, data);
                    uart_irq_tx_enable(uart_wlr089u0);
            ret_val = 1;
        } else {
            ret_val = -1;
        }
        return ret_val;
    }
    else return 1;
}


void wlr089u0_shutdown( void ) {
	gpio_pin_set(gpio_wlr089u0_nen, WLR089U0_nEN_GPIO_PIN, 1);
}

fifo_gen_buffer_t* get_rx_buffer()
{
    return &rx_buf;
}
fifo_gen_buffer_t* get_tx_buffer()
{
    return &tx_buf;
}

/*****************************************************************************************************************************
 * LoRaWAN Parser functions, 
 * For use with https://github.com/MicrochipTech/atsamr34_lorawan_rn_parser/blob/master/02_command_guide/README.md#top
 ***************************************************************************************************************************** */
/*
WLR_RESPONSE_CODE wlr_sys_sleep_ms(uint32_t ms);
WLR_RESPONSE_CODE wlr_sys_reset();
WLR_RESPONSE_CODE wlr_sys_factory_reset();
WLR_RESPONSE_CODE wlr_sys_set_customparam(uint32_t param);
int wlr_sys_get_customparam();
void wlr_sys_get_ver(char* charstring); //use strcpy
int wlr_sys_get_hweui();*/

