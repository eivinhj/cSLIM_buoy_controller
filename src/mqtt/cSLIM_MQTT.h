#ifndef CSLIM_MQTT_H
#define CSLIM_MQTT_H

#include <stdint.h>
#include <zephyr.h>
/**
 * @brief Initialize LTE and connect to MQTT broker
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_mqtt_init();

/**
 * @brief MQTT loop to be scheduled continously
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_mqtt_loop();

/**
 * @brief Publis data to MQTT Broker
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_mqtt_data_publish(uint8_t *data, size_t len);

/**
 * @brief Disconnect MQTT Client
 * @details 
 *
 * @param[in] void		void
 */
int cSLIM_mqtt_disconnect();

#endif //CSLIM_MQTT_H