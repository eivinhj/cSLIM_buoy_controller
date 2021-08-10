/*
 * cSLIM_button.c
 *
 * Created: 24.04.21
 *  Author: eivinhj
 */ 

#include "cSLIM_button.h"

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <stdint.h>
#include <logging/log.h>

#define SLEEP_TIME_MS	1

/*
 * Zephyr devices
 */
#define BTN1_NODE	DT_ALIAS(sw0)
#define BTN2_NODE	DT_ALIAS(sw1)

#if DT_NODE_HAS_STATUS(BTN1_NODE, okay)
#define BTN1_GPIO_LABEL	DT_GPIO_LABEL(BTN1_NODE, gpios)
#define BTN1_GPIO_PIN	DT_GPIO_PIN(BTN1_NODE, gpios)
#define BTN1_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(BTN1_NODE, gpios))
#else
#error "Unsupported board: BTN1 devicetree alias is not defined"
#define BTN1_GPIO_LABEL	""
#define BTN1_GPIO_PIN	0
#define BTN1_GPIO_FLAGS	0
#endif

#if DT_NODE_HAS_STATUS(BTN2_NODE, okay)
#define BTN2_GPIO_LABEL	DT_GPIO_LABEL(BTN2_NODE, gpios)
#define BTN2_GPIO_PIN	DT_GPIO_PIN(BTN2_NODE, gpios)
#define BTN2_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(BTN2_NODE, gpios))
#else
#error "Unsupported board: BTN2 devicetree alias is not defined"
#define BTN2_GPIO_LABEL	""
#define BTN2_GPIO_PIN	0
#define BTN2_GPIO_FLAGS	0
#endif

static struct gpio_callback btn1_cb_data;
static struct gpio_callback btn2_cb_data;
const struct device *btn1;
const struct device *btn2;

/* End of Zephyr devices*/


void button1_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void button2_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}


uint8_t cSLIM_button_init(button_t btn)
{
	int ret = 0;
	switch(btn)
	{
		case BTN1:
			btn1 = device_get_binding(BTN1_GPIO_LABEL);
			if (btn1 == NULL) 
			{
				printk("Error: didn't find %s device\n", BTN1_GPIO_LABEL);
				ret = 1;
				return ret;
			}
			ret = gpio_pin_configure(btn1, BTN1_GPIO_PIN, BTN1_GPIO_FLAGS);
			if (ret != 0) 
			{
				printk("Error %d: failed to configure %s pin %d\n", ret, BTN1_GPIO_LABEL, BTN1_GPIO_PIN);
				return ret;
			}
			ret = gpio_pin_interrupt_configure(btn1,
					BTN1_GPIO_PIN,
					GPIO_INT_EDGE_TO_ACTIVE);
			if (ret != 0) 
			{
				printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, BTN1_GPIO_LABEL, BTN1_GPIO_PIN);
				ret = 1;
				return ret;
			}
			gpio_init_callback(&btn1_cb_data, button1_pressed, BIT(BTN1_GPIO_PIN));
			gpio_add_callback(btn1, &btn1_cb_data);
			printk("Set up button at %s pin %d\n", BTN1_GPIO_LABEL, BTN1_GPIO_PIN);
			break;
	case BTN2:
			btn2 = device_get_binding(BTN2_GPIO_LABEL);
			if (btn2 == NULL) 
			{
				printk("Error: didn't find %s device\n", BTN2_GPIO_LABEL);
				ret = 1;
				return ret;
			}
			ret = gpio_pin_configure(btn2, BTN2_GPIO_PIN, BTN2_GPIO_FLAGS);
			if (ret != 0) 
			{
				printk("Error %d: failed to configure %s pin %d\n", ret, BTN2_GPIO_LABEL, BTN2_GPIO_PIN);
				return ret;
			}
			ret = gpio_pin_interrupt_configure(btn2,
					BTN2_GPIO_PIN,
					GPIO_INT_EDGE_TO_ACTIVE);
			if (ret != 0) 
			{
				printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, BTN2_GPIO_LABEL, BTN2_GPIO_PIN);
				ret = 1;
				return ret;
			}
			gpio_init_callback(&btn2_cb_data, button2_pressed, BIT(BTN2_GPIO_PIN));
			gpio_add_callback(btn2, &btn2_cb_data);
			printk("Set up button at %s pin %d\n", BTN2_GPIO_LABEL, BTN2_GPIO_PIN);
			break;
		default:
			printk("Error: No valid device\n");
			break;
	}
	return 0;

}

uint8_t button_interrupt_disable()
{
	gpio_remove_callback(btn1, &btn1_cb_data);
	gpio_remove_callback(btn2, &btn2_cb_data);
	return 0;
}

uint8_t button_interrupt_enable()
{
	gpio_add_callback(btn1, &btn1_cb_data);
	gpio_add_callback(btn2, &btn2_cb_data);
	return 0; 
}