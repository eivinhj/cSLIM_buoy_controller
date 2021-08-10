/*
 * cSLIM_SPI.c
 *
 * Created: 25.04.21
 *  Author: eivinhj
 * 
 * Based on Zephyr FAT FS sample
 */ 

#include "cSLIM_sd.h"

#include <stdio.h>

#include <zephyr.h>
#include <drivers/gpio.h>
#include <device.h>
#include <disk/disk_access.h>
#include <fs/fs.h>
#include <ff.h>

#include "../drivers/cSLIM_SPI.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(cSLIM_usd, LOG_LEVEL_DBG);


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

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};


static const char *disk_mount_pt = "/SD:";

//////////////////////////////////////////////////////////
// Local function declarations
/////////////////////////////////////////////////////////
int 	sd_card_mount( void );

int 	sd_card_unmount( void );

//void send_cmd(uint8_t cmd);

int	sd_card_init( void )
{
	int ret = 0;

	gpio_sd_en = device_get_binding(GPIO_SD_EN_GPIO_LABEL);
	if (gpio_sd_en == NULL) {
	printk("Didn't find GPIO device %s\n", GPIO_SD_EN_GPIO_LABEL);
	return 0;
	}
	ret = gpio_pin_configure(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, GPIO_SD_EN_GPIO_FLAGS);
	if (ret != 0) {
	printk("Error %d: failed to configure GPIO device %s pin %d\n",
			ret, GPIO_SD_EN_GPIO_LABEL, GPIO_SD_EN_GPIO_PIN);
	return ret;
	}

	//SET GPIO
	sd_card_on();

	cSLIM_spi_cs_select(SD_CARD);
	/* raw disk i/o */
	do {
		static const char *disk_pdrv = "SDHC1";
		uint64_t memory_size_mb;
		uint32_t block_count;
		uint32_t block_size;

		if (disk_access_init(disk_pdrv) != 0) {
			LOG_ERR("Storage init ERROR!");
			break;
		}

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
			LOG_ERR("Unable to get sector count");
			break;
		}
		LOG_INF("Block count %u", block_count);

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
			LOG_ERR("Unable to get sector size");
			break;
		}
		printk("Sector size %u\n", block_size);

		memory_size_mb = (uint64_t)block_count * block_size;
		printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));
	} while (0);
	


	mp.mnt_point = disk_mount_pt;
	cSLIM_spi_cs_release(SD_CARD);

	sd_card_mount();
	
return 0;

}

int 	sd_card_mount( void )
{
	cSLIM_spi_cs_select(SD_CARD);
	int res = 0;
	res = fs_mount(&mp);
	if (res == FR_OK) {
		printk("Disk mounted.\n");
	} else {
		printk("Error mounting disk.\n");
	}
	cSLIM_spi_cs_release(SD_CARD);
	return res;
}

int 	sd_card_unmount( void )
{
		fs_unmount(&mp);
	return 0; 
}

int 	sd_card_on( void )
{
	gpio_pin_set(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, 0);
	k_sleep(K_MSEC(1500));
	sd_card_mount();
	return 0; 
}

void 	sd_card_off( void )
{
	sd_card_unmount();
	gpio_pin_set(gpio_sd_en, GPIO_SD_EN_GPIO_PIN, 1);

}

int 	sd_card_write(const char* file_name, char *write_buf, uint32_t num_bytes)
{
	int err = 0 ; 
	char full_file[255];
	snprintf(full_file, sizeof(full_file), "%s/%s", disk_mount_pt,file_name);
	struct fs_file_t file;
	//fs_file_t_init(&file);
	file.mp = &mp;
	fs_mode_t flags = FS_O_WRITE | FS_O_CREATE | FS_O_APPEND;

	cSLIM_spi_cs_select(SD_CARD);
	err = fs_open(&file, full_file, flags);
	if (err) {
		printk("Error opening file %s [%d]\n", full_file, err);
		cSLIM_spi_cs_release(SD_CARD);
		return err;
	}
	err = fs_write(&file, write_buf, num_bytes);
	if (err < 0) {
		printk("Error writing file %s [%d]\n", full_file, err);
		cSLIM_spi_cs_release(SD_CARD);
		return err;
	}
	err = fs_close(&file);
	if (err) {
		printk("Error closing file %s [%d]\n", full_file, err);
		cSLIM_spi_cs_release(SD_CARD);
		return err;
	}

	cSLIM_spi_cs_release(SD_CARD);
	return err; 
}

int 	sd_card_read(const char* file_name, char *read_buf, uint32_t num_bytes)
{
	cSLIM_spi_cs_select(SD_CARD);
	//todo
	cSLIM_spi_cs_release(SD_CARD);
	return 0;
}
