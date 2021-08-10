/*
 * cSLIM_LED.c
 *
 * Created: 25.04.21
 *  Author: eivinhj
 */ 

#include "cSLIM_LED.h"


#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <stdint.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(cSLIM_gpio, LOG_LEVEL_INF);

#define SLEEP_TIME_MS	1

/*
 * Zephyr devices
 */
#define LED_RED_NODE	DT_ALIAS(led0)
#define LED_YELLOW_NODE	DT_ALIAS(led1)
#define LED_GREEN_NODE	DT_ALIAS(led2)
#define LED_BLUE_NODE	DT_ALIAS(led3)

#if DT_NODE_HAS_STATUS(LED_RED_NODE, okay) && DT_NODE_HAS_PROP(LED_RED_NODE, gpios)
#define LED_RED_GPIO_LABEL	DT_GPIO_LABEL(LED_RED_NODE, gpios)
#define LED_RED_GPIO_PIN	DT_GPIO_PIN(LED_RED_NODE, gpios)
#define LED_RED_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED_RED_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(LED_YELLOW_NODE, okay) && DT_NODE_HAS_PROP(LED_YELLOW_NODE, gpios)
#define LED_YELLOW_GPIO_LABEL	DT_GPIO_LABEL(LED_YELLOW_NODE, gpios)
#define LED_YELLOW_GPIO_PIN	DT_GPIO_PIN(LED_YELLOW_NODE, gpios)
#define LED_YELLOW_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED_YELLOW_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(LED_GREEN_NODE, okay) && DT_NODE_HAS_PROP(LED_GREEN_NODE, gpios)
#define LED_GREEN_GPIO_LABEL	DT_GPIO_LABEL(LED_GREEN_NODE, gpios)
#define LED_GREEN_GPIO_PIN	DT_GPIO_PIN(LED_GREEN_NODE, gpios)
#define LED_GREEN_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED_GREEN_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(LED_BLUE_NODE, okay) && DT_NODE_HAS_PROP(LED_BLUE_NODE, gpios)
#define LED_BLUE_GPIO_LABEL	DT_GPIO_LABEL(LED_BLUE_NODE, gpios)
#define LED_BLUE_GPIO_PIN	DT_GPIO_PIN(LED_BLUE_NODE, gpios)
#define LED_BLUE_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED_BLUE_NODE, gpios))
#endif

const struct device *led_red;
const struct device *led_yellow;
const struct device *led_green;
const struct device *led_blue;

/* End of Zephyr devices*/

int cSLIM_led_init(led_t led)
{
	int ret = 0;
	switch (led)
	{
	case RED:
		led_red = device_get_binding(LED_RED_GPIO_LABEL);
		if (led_red == NULL) {
		printk("Didn't find LED device %s\n", LED_RED_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(led_red, LED_RED_GPIO_PIN, LED_RED_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, LED_RED_GPIO_LABEL, LED_RED_GPIO_PIN);
		return ret;
		}
		break;

	case YELLOW:
		led_yellow = device_get_binding(LED_YELLOW_GPIO_LABEL);
		if (led_yellow == NULL) {
		printk("Didn't find LED device %s\n", LED_YELLOW_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(led_yellow, LED_YELLOW_GPIO_PIN, LED_YELLOW_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, LED_YELLOW_GPIO_LABEL, LED_YELLOW_GPIO_PIN);
		return ret;
	}
		break;

	case GREEN:
		led_green = device_get_binding(LED_GREEN_GPIO_LABEL);
		if (led_green == NULL) {
		printk("Didn't find LED device %s\n", LED_GREEN_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(led_green, LED_GREEN_GPIO_PIN, LED_GREEN_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, LED_GREEN_GPIO_LABEL, LED_GREEN_GPIO_PIN);
		return ret;
	}
		break;
		case BLUE:
		led_blue = device_get_binding(LED_BLUE_GPIO_LABEL);
		if (led_blue == NULL) {
		printk("Didn't find LED device %s\n", LED_BLUE_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(led_blue, LED_BLUE_GPIO_PIN, LED_BLUE_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, LED_BLUE_GPIO_LABEL, LED_BLUE_GPIO_PIN);
		return ret;
	}
		break;
	default:
		break;
	}
	return ret;
}
int cSLIM_led_on(led_t led)
{
	int ret = 0;
	switch (led)
	{
	case RED:
		gpio_pin_set(led_red, LED_RED_GPIO_PIN, 0);
		break;
	case YELLOW:
		gpio_pin_set(led_yellow, LED_YELLOW_GPIO_PIN, 0);
		break;
	case GREEN:
		gpio_pin_set(led_green, LED_GREEN_GPIO_PIN, 0);
		break;
	case BLUE:
		gpio_pin_set(led_blue, LED_BLUE_GPIO_PIN, 0);
		break;
	
	
	default:
		break;
	}

	return ret;
}
int cSLIM_led_off(led_t led)
{
	int ret = 0;
switch (led)
	{
	case RED:
		gpio_pin_set(led_red, LED_RED_GPIO_PIN, 1);
		break;
	case YELLOW:
		gpio_pin_set(led_yellow, LED_YELLOW_GPIO_PIN, 1);
		break;
	case GREEN:
		gpio_pin_set(led_green, LED_GREEN_GPIO_PIN, 1);
		break;
	case BLUE:
		gpio_pin_set(led_blue, LED_BLUE_GPIO_PIN, 1);
		break;
	
	default:
		break;
	}
	return ret;
}