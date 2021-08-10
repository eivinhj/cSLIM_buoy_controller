#ifndef cSLIM_GPIO_H
#define CSLIM_GPIO_H
#include <stdint.h>
#include <nrfx_gpiote.h>
#include <devicetree/gpio.h>

typedef enum 
{
	RED,
	GREEN,
	YELLOW,
	BLUE
} led_t;

/**
 * @brief Initialize LED
 * @details 
 *
 * @param[in] led		Led color
 */
int cSLIM_led_init(led_t led);

/**
 * @brief Turn LED on
 * @details 
 *
 * @param[in] led		Led color
 */
int cSLIM_led_on(led_t led);

/**
 * @brief Turn LED off
 * @details 
 *
 * @param[in] led		Led color
 */
int cSLIM_led_off(led_t led);


#endif //CSLIM_GPIO_H