/*
 * cSLIM_sd.c
 *
 *  Created on: Apr 19, 2017
 *      Author: Waseemh
 * 
 *  Adapted for nRF: 25.04.21
 *  Author: eivinhj
 */ 

#include <stdbool.h>
#include <zephyr.h>
#include <logging/log.h>
#include <drivers/gpio.h>
#include "cSLIM_sd.h"
#include "../drivers/cSLIM_SPI.h"
LOG_MODULE_REGISTER(cSLIM_SD);

//TODO Change to read write file

/*
 * Zephyr devices
 */
#define GPIO_SD_EN_NODE	DT_ALIAS(sdenable)

#if DT_NODE_HAS_STATUS(GPIO_SD_EN_NODE, okay) && DT_NODE_HAS_PROP(GPIO_SD_EN_NODE, gpios)
#define GPIO_SD_EN_GPIO_LABEL	DT_GPIO_LABEL(GPIO_SD_EN_NODE, gpios)
#define GPIO_SD_EN_GPIO_PIN	DT_GPIO_PIN(GPIO_SD_EN_NODE, gpios)
#define GPIO_SD_EN_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(GPIO_SD_EN_NODE, gpios))
#endif

const struct device *gpio_sd_en;

/* End of Zephyr devices*/


static	uint8_t 	sector_erase_size=0;
/*
 * private functions
 */
void start_transfer( void ) {
	cSLIM_spi_cs_select(SD_CARD);
}


void end_transfer( void ) {
	cSLIM_spi_cs_release(SD_CARD);
	send_cmd(0xFF);
}

void send_cmd(uint8_t cmd) {
	switch (cmd) {
        case CMD_0:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(CRC_0);
            break;

        case CMD_1:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_1);
            cSLIM_spi_write_byte(0x40);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(0xff);//ARG_0
            break;

        case CMD_8:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_8);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_1);
            cSLIM_spi_write_byte(ARG_AA);
            cSLIM_spi_write_byte(0x87);
            break;

        case CMD_16:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_16);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(0x02);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(0x87);
            break;

        case CMD_41:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_41);
            cSLIM_spi_write_byte(0x40);	//0x40
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(0x41);	//Modification for Kingston from ARG_0...
            cSLIM_spi_write_byte(0x77);	//0x77 for 0x40
            break;

        case CMD_55:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_55);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(0x65);
            break;


        case CMD_58:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_58);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(CRC_58);
            break;


        case CMD_23:
            cSLIM_spi_write_byte(0xFF);
            cSLIM_spi_write_byte(CMD_23);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(ARG_0);
            cSLIM_spi_write_byte(sector_erase_size);
            cSLIM_spi_write_byte(0xff);
            break;

        default:
            cSLIM_spi_write_byte(0xFF);
            break;
	}
}


void set_sector_size(uint8_t size) {
	uint8_t retry=0;
    // ACMD_23 and response
	start_transfer();
	send_cmd(CMD_55);
	retry=0;
	char reply;
	cSLIM_spi_read_write_byte(0xFF, &reply);
	while(reply==0xff){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		retry++;
		if(retry>500){
			end_transfer();
			return;
		}
	}
	end_transfer();
	sector_erase_size=size;
	start_transfer();
	send_cmd(CMD_23);
	retry=0;
	cSLIM_spi_read_write_byte(0xFF, &reply);
	while(reply==0xff){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		retry++;
		if(retry>500){
			end_transfer();
			return;
		}
	}
	end_transfer();
}


/*
 * public functions
 */

int sd_card_init( void ) {
	int 	outer_loop_var=0;
	int 	inner_loop_var=0;
	uint8_t	reply=0;

	int ret = 0;

	gpio_sd_en = device_get_binding(GPIO_SD_EN_GPIO_LABEL);
		if (gpio_sd_en == NULL) {
		printk("Didn't find LED device %s\n", GPIO_SD_EN_GPIO_LABEL);
		return 0;
		}
		ret = gpio_pin_configure(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, GPIO_SD_EN_GPIO_FLAGS);
		if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n",
		       ret, GPIO_SD_EN_GPIO_LABEL, GPIO_SD_EN_GPIO_PIN);
		return ret;
		}


	sd_card_on();
	start_transfer();		//to set all other...
	end_transfer();
		//power on delay
	k_sleep(K_MSEC(7));
		//80 dummy cycles
	for(outer_loop_var=0; outer_loop_var<10; ++outer_loop_var) {
		send_cmd(0xFF);
	}
		//CMD_0 and response
	start_transfer();
	send_cmd(CMD_0);
	reply=0xFF;
	while(reply!=0x01){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=100){
			end_transfer();
			return false;
			break;
		}
	}
	end_transfer();
		//CMD_8 and response
	start_transfer();
	send_cmd(CMD_8);
	reply=0xFF;
	outer_loop_var=0;
	while(reply!=0x01){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=100){
			end_transfer();
			return false;
			break;
		}
	}
	cSLIM_spi_read_write_byte(0xFF, &reply);
	cSLIM_spi_read_write_byte(0xFF, &reply);
	cSLIM_spi_read_write_byte(0xFF, &reply);
	cSLIM_spi_read_write_byte(0xFF, &reply);
	if (reply!=0xAA){
		end_transfer();
		return false;
	}
	end_transfer();
		//ACMD_41 and response
	reply=0xFF;
	while(reply!=0x00){
		start_transfer();
		send_cmd(CMD_55);
		cSLIM_spi_read_write_byte(0xFF, &reply);		//Modification for Kingston...
		end_transfer();
		start_transfer();
		send_cmd(CMD_41);
		inner_loop_var=0;
		while(reply!=0x00){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			inner_loop_var++;
			if(inner_loop_var>3){break;}
		}
		//k_sleep(K_MSEC(7));
		outer_loop_var++;
		if(outer_loop_var>=100){
			end_transfer();
			return false;
			break;
		}
	}
	end_transfer();
		//CMD_58 and CCS bit
	start_transfer();
	send_cmd(CMD_58);
	reply=0xFF;
	outer_loop_var=0;
	while(reply!=0x00){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=100){
			end_transfer();
			return false;
			break;
		}
	}
	cSLIM_spi_read_write_byte(0xFF, &reply);
	if (reply!=0xC0){
		end_transfer();
		return false;
	}
	cSLIM_spi_read_write_byte(0xFF, &reply);
	cSLIM_spi_read_write_byte(0xFF, &reply);
	cSLIM_spi_read_write_byte(0xFF, &reply);
	end_transfer();
	return true;
}


void sd_card_on( void ) {
	gpio_pin_set(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, 0);
}


void sd_card_off( void ) {
	gpio_pin_set(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, 1);
}


bool sd_card_read(uint32_t addr, char *read_buf,uint32_t scetor_count) {
	int 		outer_loop_var=0;
	int 		inner_loop_var=0;
	uint32_t	offset=0;
	uint8_t		reply=0;
	bool 		flag=true;
	uint32_t	retry=0;

	start_transfer();
	cSLIM_spi_write_byte(0xFF);
	cSLIM_spi_write_byte(CMD_18);
	cSLIM_spi_write_byte((uint8_t)(addr>>24));
	cSLIM_spi_write_byte((uint8_t)(addr>>16));
	cSLIM_spi_write_byte((uint8_t)(addr>>8));
	cSLIM_spi_write_byte((uint8_t)(addr>>0));
	cSLIM_spi_write_byte(0xFF);	//CRC
	reply=0xFF;
	outer_loop_var=0;
	while(reply!=0x00){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=10){
			flag =false;
			break;
		}
	}
	offset=0;
	if(flag==true){
		for(outer_loop_var=0;outer_loop_var<scetor_count;outer_loop_var++){
			inner_loop_var=0;
			reply=0xFF;
			while(reply!=0xFE){
				cSLIM_spi_read_write_byte(0xFF, &reply);
				inner_loop_var++;
				if(inner_loop_var>=100){
					flag =false;
					break;
					cSLIM_spi_write_byte(CMD_12);
					cSLIM_spi_write_byte((uint8_t)(ARG_0));
					cSLIM_spi_write_byte((uint8_t)(ARG_0));
					cSLIM_spi_write_byte((uint8_t)(ARG_0));
					cSLIM_spi_write_byte((uint8_t)(ARG_0));
					cSLIM_spi_write_byte(0xFF);	//CRC
					end_transfer();
					return flag;
				}
			}
				//actual packet
			for(inner_loop_var=0;inner_loop_var<SD_CARD_BLOCK_SIZE+2;inner_loop_var++){
				if(inner_loop_var<SD_CARD_BLOCK_SIZE){
				cSLIM_spi_read_write_byte(0xFF, &(read_buf[inner_loop_var+offset]));
				}
				else{
					cSLIM_spi_read_write_byte(0xFF, NULL);	//discard CRC
				}
			}
			offset=SD_CARD_BLOCK_SIZE;
		}

		cSLIM_spi_write_byte(CMD_12);
		cSLIM_spi_write_byte((uint8_t)(ARG_0));
		cSLIM_spi_write_byte((uint8_t)(ARG_0));
		cSLIM_spi_write_byte((uint8_t)(ARG_0));
		cSLIM_spi_write_byte((uint8_t)(ARG_0));
		cSLIM_spi_write_byte(0xFF);	//CRC

			retry=0;
		char reply;
		cSLIM_spi_read_write_byte(0xFF, &reply);
		while(reply!=0x00){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			retry++;
			if(retry>500){
				flag=false;
				break;
			}
		}	//1-8 bytes and afterwards 00
		cSLIM_spi_read_write_byte(0xFF, NULL);
		retry=0;
		cSLIM_spi_read_write_byte(0xFF, &reply);
		while(reply!=0xFF){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			retry++;
			if(retry>500){
				flag=false;
				break;
			}
		}	//afterwards 0xff

	}
	end_transfer();
/*	for(inner_loop_var=0;inner_loop_var<scetor_count;inner_loop_var++){
		start_transfer();
		cSLIM_spi_write_byte(0xFF);
		cSLIM_spi_write_byte(CMD_17);
		cSLIM_spi_write_byte((uint8_t)(addr>>24));
		cSLIM_spi_write_byte((uint8_t)(addr>>16));
		cSLIM_spi_write_byte((uint8_t)(addr>>8));
		cSLIM_spi_write_byte((uint8_t)(addr>>0));
		cSLIM_spi_write_byte(0xFF);	//CRC
		reply=0xFF;
		outer_loop_var=0;
		while(reply!=0x00){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			outer_loop_var++;
			if(outer_loop_var==10){
				flag =false;
				break;
			}
		}
		reply=0xFF;
		outer_loop_var=0;
		while(reply!=0xFE){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			outer_loop_var++;
			if(outer_loop_var==100){
				flag =false;
				break;
			}
		}
		if (flag==true){
			for(outer_loop_var=0;outer_loop_var<SD_CARD_BLOCK_SIZE+2;outer_loop_var++){
				if(outer_loop_var<SD_CARD_BLOCK_SIZE){
				cSLIM_spi_read_write_byte(0xFF, &(read_buf[outer_loop_var]));
				}
				else{
					cSLIM_spi_read_write_byte(0xFF, NULL);	//discard CRC
				}
			}
		}
		end_transfer();
		addr++;
	}
	*/
	return flag;
}


bool sd_card_write(uint32_t addr, char *write_buf,uint32_t scetor_count) {
	int 	outer_loop_var=0;
	int 	inner_loop_var=0;
	uint8_t	reply=0;
	bool 	flag=true;
	uint8_t	retry=0;

	set_sector_size((uint8_t)scetor_count);
	//for(inner_loop_var=0;inner_loop_var<scetor_count;inner_loop_var++){
		start_transfer();
		cSLIM_spi_write_byte(0xFF);
		cSLIM_spi_write_byte(CMD_25);
		cSLIM_spi_write_byte((uint8_t)(addr>>24));
		cSLIM_spi_write_byte((uint8_t)(addr>>16));
		cSLIM_spi_write_byte((uint8_t)(addr>>8));
		cSLIM_spi_write_byte((uint8_t)(addr>>0));
		cSLIM_spi_write_byte(0xFF);	//CRC
		reply=0xFF;
		outer_loop_var=0;
			//CMD response
		while(reply!=0x00){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			outer_loop_var++;
			if(outer_loop_var>=100){
				flag =false;
				break;
			}
		}
		//////////////////////
		cSLIM_spi_write_byte(0xFF);		//1 byte gap
		cSLIM_spi_write_byte(DATA_TKN_25);		//???
			//write data packet
		retry=0;
		if(flag==true){
			for(inner_loop_var=0;inner_loop_var<scetor_count;inner_loop_var++){
					//actual packet
				for(outer_loop_var=0;outer_loop_var<SD_CARD_BLOCK_SIZE+2;outer_loop_var++){
					cSLIM_spi_read_write_byte((uint8_t)write_buf[outer_loop_var], NULL);
					}
					//repnose
				while(1){
					cSLIM_spi_read_write_byte(0xFF, &reply);
					if(reply==0xFF){
						break;
					}
					else{
						retry++;
						if(retry>500){
							flag=false;
							break;
						}
					}
				}
			}
		}
		cSLIM_spi_write_byte(STOP_TOKEN);		//???
		cSLIM_spi_write_byte(0xFF);		//???
		retry=0;
		cSLIM_spi_read_write_byte(0xFF, &reply);
		while(!reply){
			cSLIM_spi_read_write_byte(0xFF, &reply);
			retry++;
			if(retry>500){
				flag=false;
				break;
			}
		}
		end_transfer();
		//addr++;
	//}
	return flag;
}


bool sd_card_specs(char *csd) {
	int 	outer_loop_var=0;
	uint8_t	reply=0;
	bool 	flag=true;
		//CMD17 and response
	start_transfer();
	cSLIM_spi_write_byte(0xFF);
	cSLIM_spi_write_byte(CMD_9);
	cSLIM_spi_write_byte(0x00);
	cSLIM_spi_write_byte(0x00);
	cSLIM_spi_write_byte(0x00);
	cSLIM_spi_write_byte(0x00);
	cSLIM_spi_write_byte(0xFF);	//CRC
	reply=0xFF;
	outer_loop_var=0;
	while(reply!=0x00){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=10){
			flag =false;
			break;
		}
	}
	reply=0xFF;
	outer_loop_var=0;
	while(reply!=0xFE){
		cSLIM_spi_read_write_byte(0xFF, &reply);
		outer_loop_var++;
		if(outer_loop_var>=10){
			flag =false;
			break;
		}
	}
	if (flag==true){
		for(outer_loop_var=0;outer_loop_var<18;outer_loop_var++){
			cSLIM_spi_read_write_byte(0xFF, &(csd[outer_loop_var]));
		}
	}
	end_transfer();
	return flag;
}
