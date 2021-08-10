/*
 * display.c
 *
 *  Created on: 6. sep. 2018
 *      Author: mvols
 *
 *  Expanded on: Feb 2020
 *      Author: MariusSR
 * 	
 *  Ported to nRF: Apr 2021
 *      Author: Eivinhj
 *
 */

#include "display.h"
#include "../ugui/ugui.h"

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <stdint.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(display, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

#include "../drivers/cSLIM_SPI.h"


uint8_t bytes_to_write[(DISPLAY_LINES*(DISPLAY_DOTS_PER_LINE / 8) + 2*DISPLAY_LINES + 2)];
// Display buffer for storing the entire frame
uint8_t display_buffer[DISPLAY_LINES][DISPLAY_DOTS_PER_LINE / 8];


// COM inversion variable
static uint8_t com_value;

// Required by graphical library
UG_GUI gui;

void UserSetPixel (UG_S16 x, UG_S16 y, UG_COLOR c);

void UserSetPixel (UG_S16 x, UG_S16 y, UG_COLOR c) {
	if (x > DISPLAY_DOTS_PER_LINE)
		x = DISPLAY_DOTS_PER_LINE - 1;
	if (y > DISPLAY_LINES)
		y = DISPLAY_LINES - 1;
    display_draw(c == 0, (uint8_t)x, (uint8_t)y);
}


void display_init( void ) {
	// Initialize SPI and clear display
	//cSLIM_spi_init();
	//cSLIM_CS_init(DISPLAY);
	com_value = 0;

	display_clear_and_update();

	// Setup graphical library
	UG_Init(&gui, UserSetPixel, DISPLAY_DOTS_PER_LINE, DISPLAY_LINES);
	UG_FillScreen(C_WHITE);
	UG_FontSelect(&FONT_12X16);
	UG_SetBackcolor(C_WHITE);
	UG_SetForecolor(C_BLACK);
}


void display_put_string(uint8_t x, uint8_t y, unsigned char* data, font_size_t size) {
	switch (size) {
        case font_small:
//    		UG_FontSelect(&FONT_5X8);
            UG_FontSelect(&FONT_6X8);
            break;
        case font_medium:
            UG_FontSelect(&FONT_8X12);
            break;
        case font_large:
        default:
            UG_FontSelect(&FONT_12X16);
	}
	UG_PutString(x, y, data);
}


void display_draw(uint8_t value, uint8_t x, uint8_t y) {
	// Rotate by 90 degrees clockwise
	uint8_t old_x = x;
	uint8_t old_y = y;

	x = DISPLAY_LINES - 1 - old_y;
	y = old_x;

	// Set pixel in display buffer
	if (value) display_buffer[y][x>>3] &= ~(1<<(x&0x7));
	else       display_buffer[y][x>>3] |=  (1<<(x&0x7));
}


void display_draw_horisontal_line(uint8_t y, uint8_t x_start, uint8_t x_end) {
    for (int x = x_start; x < x_end+1; ++x) display_draw(1, x, y);
}


void display_draw_horisontal_whole_line(uint8_t y) {
    display_draw_horisontal_line(y, 0, 128);
}


void display_draw_vertical_line(uint8_t x, uint8_t y_start, uint8_t y_end) {
    for (int y = y_start; y < y_end; ++y) display_draw(1, x, y);
}


void display_draw_vertical_whole_line(uint8_t x) {
    display_draw_vertical_line(x, 0, 128);
}


void display_clear( void ) {
	// Clear display buffer contents
	for (uint32_t line = 0; line < DISPLAY_LINES; line++)
		for (uint32_t dots = 0; dots < (DISPLAY_DOTS_PER_LINE / 8); dots++)
			display_buffer[line][dots] = 0xFF;
}


void display_clear_lines_from(uint8_t from_line) {
    // Clear display buffer contents
    for (uint32_t line = 0; line < DISPLAY_LINES; line++)
        for (uint32_t dots = 0; dots < (DISPLAY_DOTS_PER_LINE / 8) - from_line; dots++)
            display_buffer[line][dots] = 0xFF;
}


void display_clear_main_area( void ) {
    display_clear_lines_from(2);
}


void display_toggle_com( void ) {
	// Toggle COM
	com_value ^= DISPLAY_COM;
	uint8_t cmd = (com_value & DISPLAY_COM);

	// Dummy command, only updates COM
	cSLIM_spi_cs_select(DISPLAY);
	// LS013B7DH03 uses LSB first SPI communication.
	cSLIM_spi_set_byte_order(LSBF);

	cSLIM_spi_write_byte(cmd);

	cSLIM_spi_set_byte_order(MSBF);
	cSLIM_spi_cs_release(DISPLAY);
}


void display_clear_and_update( void ) {
	display_clear();
	//display_update();

	// Toggle COM

	com_value ^= DISPLAY_COM;
	uint8_t cmd = DISPLAY_ALL_CLEAR | (com_value & DISPLAY_COM);

	// Clear display
	cSLIM_spi_cs_select(DISPLAY);

	// LS013B7DH03 uses LSB first SPI communication.
	cSLIM_spi_set_byte_order(LSBF);
uint8_t bytes[4];
	bytes[0] = cmd;
	bytes[1] = 0x00;
	bytes[2] = 0x00;
	bytes[3] = 0x00;	
	cSLIM_spi_write_bytes(bytes,4);


	cSLIM_spi_set_byte_order(MSBF);
	cSLIM_spi_cs_release(DISPLAY);

}


void display_update( void ) {
    display_activity_dot_outline();
    display_activity_dot_toggle();
	// Toggle COM
	com_value ^= DISPLAY_COM;
	uint8_t cmd = DISPLAY_UPDATE_FLAG | (com_value & DISPLAY_COM);
	
	bytes_to_write[0] = cmd;
	uint16_t bytes_to_write_index = 1;
	
	for (uint32_t line = 0; line < DISPLAY_LINES; line++)
		{
			bytes_to_write[bytes_to_write_index] = (line + 1);
			bytes_to_write_index++;
			for (uint32_t dots = 0; dots < (DISPLAY_DOTS_PER_LINE / 8); dots++)
			{
				bytes_to_write[bytes_to_write_index] = (display_buffer[line][dots]);
				bytes_to_write_index++;
			}
			bytes_to_write[bytes_to_write_index] = (0x00);
			bytes_to_write_index++;
		}
	bytes_to_write[bytes_to_write_index] = (0x00);
	bytes_to_write_index++;
	// Write display buffer to display
	cSLIM_spi_cs_select(DISPLAY);
	// LS013B7DH03 uses LSB first SPI communication.
	cSLIM_spi_set_byte_order(LSBF);
	cSLIM_spi_write_bytes(bytes_to_write, bytes_to_write_index);
	// Iterate over each line
	

	cSLIM_spi_set_byte_order(MSBF);
	cSLIM_spi_cs_release(DISPLAY);

}


void draw_box(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end) {
    display_draw_horisontal_line(y_start, x_start, x_end);
    display_draw_horisontal_line(y_end, x_start, x_end);
    display_draw_vertical_line(x_start, y_start, y_end);
    display_draw_vertical_line(x_end, y_start, y_end);
}


void display_activity_dot_outline( void ) {
    draw_box(120, 120, 126, 126);
}


void display_activity_dot(bool show){
    const uint8_t from = 122;
    const uint8_t to   = 125;
    for (int i = from; i < to; ++i)
        for (int j = from; j < to; ++j)
            display_draw(show, i, j);
}


void display_activity_dot_toggle( void ) {
    static bool state = false;
    display_activity_dot(!state);
    state = !state;
}
