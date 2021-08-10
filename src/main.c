/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <drivers/flash.h>
#include <device.h>
#include <devicetree.h>
#include <string.h>
#include <random/rand32.h>
#include <net/mqtt.h>
#include <net/socket.h>
#include <modem/lte_lc.h>
#include <logging/log.h>
#include <shell/shell.h>

#if defined(CONFIG_MODEM_KEY_MGMT)
#include <modem/modem_key_mgmt.h>
#endif
#if defined(CONFIG_LWM2M_CARRIER)
#include <lwm2m_carrier.h>
#endif

#include "mqtt/certificates.h"

#include "modem.h"
#include "mqtt/cSLIM_MQTT.h"
#include <nrfx_gpiote.h>
#include <helpers/nrfx_gppi.h>
#if defined(DPPI_PRESENT)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif

#include <string.h>
#include <stdbool.h>
#include "drivers/cSLIM_button.h"
#include "drivers/cSLIM_SPI.h"
#include "drivers/cSLIM_LED.h"
#include "drivers/cSLIM_rs232.h"
#include "devices/cSLIM_analog.h"
#include "devices/rv-3032-c7.h"
#include "devices/wlr089u0.h"
#include "devices/display.h"
#include "devices/cSLIM_fram.h"
#include "devices/cSLIM_sd.h"
#include "local_log/local_log.h"

#include "tasks/LoRa_task.h"
#include "tasks/display_task.h"
#include "tasks/cslim_status_task.h"
#include "tasks/gps_task.h"
#include "tasks/time_task.h"
#include "tasks/tbr_sync_task.h"
#include "tasks/tbr_detect_task.h"
#include "tasks/mqtt_task.h"
#include <time.h>

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 1024

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

#define MQTT_ENABLE 0

LOG_MODULE_REGISTER(cslim_main, CONFIG_MQTT_SIMPLE_LOG_LEVEL);


void main(void)
{

	
	int err;
	(void) err;
	LOG_INF("Application starting %d", DT_GPIO_PIN(DT_ALIAS(sw0),gpios));

	//cSLIM_button_init(BTN1);  //TODO Some problem here makes it hang
	cSLIM_spi_init();
	cSLIM_CS_init(ALL);

	//cSLIM_button_init(BTN1); //Used by uSD card, must find a solution to share this, or use other pins if making new design
	//cSLIM_button_init(BTN2); //Used by uSD card, must find a solution to share this, or use other pins if making new design

	cSLIM_led_init(GREEN);
	cSLIM_led_on(GREEN);
	//rs232_init(); 


	k_sleep(K_SECONDS(4));

	local_log_init(LOG_FRAM);   //Initiate and put to sleep
	local_log_init(LOG_uSD);
	sd_card_off();				//Save power

	LOG_INF("MAIN CONFIG DONE");

/*	rv3032_init();
	while(1)
	{
		k_sleep(K_MSEC(1));
		rv3032_get_date_and_time();	
	}
*/

	while (1) {
			k_sleep(K_FOREVER);
			
	}
}


//Tasks

K_THREAD_DEFINE(loRa_task_id, 2*TASK_DEFAULT_STACKSIZE, loRa_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY, 0, 0);
K_THREAD_DEFINE(display_task_id, 2*TASK_DEFAULT_STACKSIZE, display_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY, 0, 0);
K_THREAD_DEFINE(tbr_sync_task_id, 2*TASK_DEFAULT_STACKSIZE, tbr_sync_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY - 2, 0, 0);
K_THREAD_DEFINE(tbr_detect_task_id, TASK_DEFAULT_STACKSIZE, tbr_detect_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY, 0, 0);
K_THREAD_DEFINE(cslim_status_task_id, TASK_DEFAULT_STACKSIZE, cslim_status_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY, 0, 0);
K_THREAD_DEFINE(mqtt_task_id, 2*TASK_DEFAULT_STACKSIZE, mqtt_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY, 0, 0);
K_THREAD_DEFINE(gps_task_id, TASK_DEFAULT_STACKSIZE, gps_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY -3, 0, 0);
K_THREAD_DEFINE(time_task_id, TASK_DEFAULT_STACKSIZE, time_task, NULL, NULL, NULL, TASK_DEFAULT_PRIORITY-1, 0, 0);