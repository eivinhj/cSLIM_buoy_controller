#ifndef CSLIM_STATUS_TASK
#define CSLIM_STATUS_TASK
#endif //CSLIM_STATUS_TASK

#include "../devices/ublox_gps.h"

#define TEMPERATURE_TRESHOLD 10

/**
 * @brief Add navigation message to status task
 * @details Used to update cSLIM position
 *
 * @retval 
 */
void cslim_status_task_add_nav_message(nav_data_t* nav_data);

/**
 * @brief Set TBR serial number for access by cSLIM status task
 * @details 
 *
 * @retval 
 */
void cslim_status_set_tbr_serial_number(uint16_t serial_number);

/**
 * @brief cSLIM status task
 * @details 
 *
 * @retval 
 */
void cslim_status_task();