#ifndef cSLIM_SWITCH_H
#define CSLIM_SWITCH_H
#include <stdint.h>
#include <nrfx_gpiote.h>
#include <devicetree/gpio.h>

typedef enum 
{
	BTN1,
	BTN2
} button_t;

/**
 * @brief Initialize buttons
 * @details Button presses will call interrupt handlers
 *
 * @param[in] void		void
 */
uint8_t cSLIM_button_init(button_t btn);


#endif //CSLIM_SWITCH_H