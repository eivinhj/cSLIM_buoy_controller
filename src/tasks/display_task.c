#include "display_task.h"

#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "../time/time_local.h"

#include "../drivers/cSLIM_SPI.h"
#include "../devices/rv-3032-c7.h"
#include "../devices/display.h"
#include "../buffers/fifo_gen.h"
LOG_MODULE_REGISTER(display_task, LOG_LEVEL_INF);

/* size of stack area used by each thread */
#define TASK_DEFAULT_STACKSIZE 128

/* scheduling priority used by each thread */
#define TASK_DEFAULT_PRIORITY 7

K_FIFO_DEFINE(display_task_fifo);

static unsigned char    display_buffer[512];
bool display_update_requested = false;


void request_display_update()
{
	display_update_requested = true;
}


void display_task(void)
{
	LOG_INF("Is thread ID %x", (int) k_current_get());
	//Wait for LoRa, but toggle in case
	k_sleep(K_MSEC(4000));

	display_init();

	//display_clear();


	sprintf(display_buffer,"cSLIM");
    display_put_string(5*8+5, 2, display_buffer,font_medium);
	display_draw_horisontal_whole_line(14);

	
	//int temp = rv3032_get_temp();
    //sprintf(display_buffer, "%dd\370C", temp/100);
    //display_put_string(3, 10*12, display_buffer, font_small);


	display_update_requested = true;

	struct tm time_local; 
	int i = 0; 
	int current_minute = 59;
	while(1)
	{
		
		i++;
		k_sleep(K_MSEC(1000));

		
		
		if(time_local_get_date_and_time_tm(&time_local) == 0) 
		{
			//update every minute
			if(time_local.tm_min != current_minute)
			{
				//Display UTC time
				sprintf(display_buffer,"%2d:%2d", time_local.tm_hour, time_local.tm_min);
				display_put_string(5*8+5, 2, display_buffer, font_medium);
				display_update_requested = true;
				current_minute =  time_local.tm_min;
			}

		}
		
		//sprintf(display_buffer, "Num: %d", i);
    	//display_put_string(10, 60, display_buffer, font_small);
		//i++;
		display_toggle_com();
		if(display_update_requested)
		{
			display_update();
			display_update_requested = false;
		}
				
		
	}
}