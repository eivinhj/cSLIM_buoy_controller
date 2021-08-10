/*
 * display.c
 *
 *  Created on: 6. sep. 2018
 *      Author: mvols
 *
 *  Expanded on: Feb 2020
 *      Author: MariusSR
 *
 */

#ifndef DEVICES_HEADER_DISPLAY_H_
#define DEVICES_HEADER_DISPLAY_H_

#include <sys/util.h>
//#include "../drivers/delay.h"

typedef enum{
	font_small=0,
	font_medium,
	font_large
}font_size_t;


#define DISPLAY_LINES 			128
#define DISPLAY_DOTS_PER_LINE	128

#define DISPLAY_UPDATE_FLAG	(1<<0)
#define DISPLAY_COM			(1<<1)
#define DISPLAY_ALL_CLEAR	(1<<2)

/**
 * @brief Initialize display
 * @details 
 *
 * @param[in] void		void
 */
void display_init( void );

/**
 * @brief Toggle display com
 * @details This must be toggled regularly for the display not to hang up
 *
 * @param[in] void		void
 */
void display_toggle_com( void );

/**
 * @brief Clear display buffer and write it to display
 * @details 
 *
 * @param[in] void		void
 */
void display_clear_and_update( void );

/**
 * @brief Update display with the content of display buffer
 * @details 
 *
 * @param[in] void		void
 */
void display_update( void );

/**
 * @brief Put string in display buffer
 * @details Starting on coordinates given, selectable font size
 *
 * @param[in] x x coordinate to start writing
 * @param[in] y y coordinate to start writing
 * @param[in] data pointer to charstring to write
 * @param[in] size font size
 */
void display_put_string(uint8_t x, uint8_t y, unsigned char* data, font_size_t size);

/**
 * @brief Draw on display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] value value to write
 * @param[in] x x coordinate to start writing
 * @param[in] y y coordinate to start writing
 */
void display_draw(uint8_t value, uint8_t x, uint8_t y);

/**
 * @brief Draw horisontal line on display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] y y coordinate the line should be drawn
 * @param[in] x_start start of line
 * @param[in] x_end end of line
 */
void display_draw_horisontal_line(uint8_t y, uint8_t x_start, uint8_t x_end);

/**
 * @brief Draw horisontal line on entire display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] y y coordinate the line should be drawn
 */
void display_draw_horisontal_whole_line(uint8_t y);

/**
 * @brief Draw vertical line on display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] x x coordinate the line should be drawn
 * @param[in] y_start start of line
 * @param[in] y_end end of line
 */
void display_draw_vertical_line(uint8_t x, uint8_t y_start, uint8_t y_end);

/**
 * @brief Draw vertical line on entire display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] x x coordinate the line should be drawn
 */
void display_draw_vertical_whole_line(uint8_t x);

/**
 * @brief Draw box on display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] x_start start of box in x-direction
 * @param[in] x_end end of box in x-direction
 * @param[in] y_start start of box in y-direction
 * @param[in] y_end end of box in y-direction
 */
void draw_box(uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end);

/**
 * @brief Display box around activity dot
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] void		void
 */
void display_activity_dot_outline( void );

/**
 * @brief Display activity dot
 * @details Used to indicate if the display have been updated.
 * Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] void		void
 */
void display_activity_dot(bool show);

/**
 * @brief Toggle activity dot
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] void		void
 */
void display_activity_dot_toggle( void );

/**
 * @brief Clear display
 * @details 
 *
 * @param[in] void		void
 */
void display_clear( void );

/**
 * @brief Clear main area of display
 * @details Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] void		void
 */
void display_clear_main_area( void );

/**
 * @brief Clear lines on display from a given starting line
 * @details Will clean every line after this.
 * Changes are only made in display buffer, for changes to show on display it must first be updated
 *
 * @param[in] from_line First line to clean
 */
void display_clear_lines_from(uint8_t from_line);

#endif /* DEVICES_HEADER_DISPLAY_H_ */
