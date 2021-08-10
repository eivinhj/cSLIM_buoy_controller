/*
 * cSLIM_modem.c
 *
 * Based on Nordic Semiconductor Example in NCS
 * 
 * Created: 25.04.21
 *  Author: eivinhj
 */ 


#include "modem.h"
#include <zephyr.h>
#include <modem/lte_lc.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(cslim_modem, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

#if defined(CONFIG_MQTT_LIB_TLS)
int certificates_provision(void)
{
	int err = 0;

	LOG_INF("Provisioning certificates");

#if defined(CONFIG_NRF_MODEM_LIB) && defined(CONFIG_MODEM_KEY_MGMT)

	err = modem_key_mgmt_write(CONFIG_MQTT_TLS_SEC_TAG,
				   MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
				   CA_CERTIFICATE,
				   strlen(CA_CERTIFICATE));
	if (err) {
		LOG_ERR("Failed to provision CA certificate: %d", err);
		return err;
	}

#elif defined(CONFIG_BOARD_QEMU_X86) && defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)

	err = tls_credential_add(CONFIG_MQTT_TLS_SEC_TAG,
				 TLS_CREDENTIAL_CA_CERTIFICATE,
				 CA_CERTIFICATE,
				 sizeof(CA_CERTIFICATE));
	if (err) {
		LOG_ERR("Failed to register CA certificate: %d", err);
		return err;
	}

#endif

	return err;
}
#endif /* defined(CONFIG_MQTT_LIB_TLS) */

#if defined(CONFIG_NRF_MODEM_LIB)

/**@brief Recoverable modem library error. */
void nrf_modem_recoverable_error_handler(uint32_t err)
{
	LOG_ERR("Modem library recoverable error: %u", (unsigned int)err);
}

#endif /* defined(CONFIG_NRF_MODEM_LIB) */

#if defined(CONFIG_LWM2M_CARRIER)
K_SEM_DEFINE(carrier_registered, 0, 1);
int lwm2m_carrier_event_handler(const lwm2m_carrier_event_t *event)
{
	switch (event->type) {
	case LWM2M_CARRIER_EVENT_BSDLIB_INIT:
		LOG_INF("LWM2M_CARRIER_EVENT_BSDLIB_INIT");
		break;
	case LWM2M_CARRIER_EVENT_CONNECTING:
		LOG_INF("LWM2M_CARRIER_EVENT_CONNECTING");
		break;
	case LWM2M_CARRIER_EVENT_CONNECTED:
		LOG_INF("LWM2M_CARRIER_EVENT_CONNECTED");
		break;
	case LWM2M_CARRIER_EVENT_DISCONNECTING:
		LOG_INF("LWM2M_CARRIER_EVENT_DISCONNECTING");
		break;
	case LWM2M_CARRIER_EVENT_DISCONNECTED:
		LOG_INF("LWM2M_CARRIER_EVENT_DISCONNECTED");
		break;
	case LWM2M_CARRIER_EVENT_BOOTSTRAPPED:
		LOG_INF("LWM2M_CARRIER_EVENT_BOOTSTRAPPED");
		break;
	case LWM2M_CARRIER_EVENT_REGISTERED:
		LOG_INF("LWM2M_CARRIER_EVENT_REGISTERED");
		k_sem_give(&carrier_registered);
		break;
	case LWM2M_CARRIER_EVENT_DEFERRED:
		LOG_INF("LWM2M_CARRIER_EVENT_DEFERRED");
		break;
	case LWM2M_CARRIER_EVENT_FOTA_START:
		LOG_INF("LWM2M_CARRIER_EVENT_FOTA_START");
		break;
	case LWM2M_CARRIER_EVENT_REBOOT:
		LOG_INF("LWM2M_CARRIER_EVENT_REBOOT");
		break;
	case LWM2M_CARRIER_EVENT_LTE_READY:
		LOG_INF("LWM2M_CARRIER_EVENT_LTE_READY");
		break;
	case LWM2M_CARRIER_EVENT_ERROR:
		LOG_ERR("LWM2M_CARRIER_EVENT_ERROR: code %d, value %d",
			((lwm2m_carrier_event_error_t *)event->data)->code,
			((lwm2m_carrier_event_error_t *)event->data)->value);
		break;
	default:
		LOG_WRN("Unhandled LWM2M_CARRIER_EVENT: %d", event->type);
		break;
	}

	return 0;
}
#endif /* defined(CONFIG_LWM2M_CARRIER) */



/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
int modem_configure(void)
{

	/* Turn on/off LTE power saving features. Some
	 * networks rejects timer updates after the device has registered to the
	 * LTE network.
	 */
	int err;
	LOG_INF("Enabling PSM");
	err = lte_lc_psm_req(true);
	if (err) {
		LOG_ERR("lte_lc_psm_req, error: %d\n", err);
	}
	LOG_INF("Enabling PSM and eDRX");
	err = lte_lc_edrx_req(true);
	if (err) {
		LOG_ERR("lte_lc_edrx_req, error: %d\n", err);
	}

#if defined(CONFIG_LTE_LINK_CONTROL)
	if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT)) {
		/* Do nothing, modem is already turned on
		 * and connected.
		 */
	} else {
#if defined(CONFIG_LWM2M_CARRIER)
		/* Wait for the LWM2M_CARRIER to configure the modem and
		 * start the connection.
		 */
		LOG_INF("Waitng for carrier registration...");
		k_sem_take(&carrier_registered, K_FOREVER);
		LOG_INF("Registered!");
#else /* defined(CONFIG_LWM2M_CARRIER) */
		int err;

		LOG_INF("LTE Link Connecting...");
		err = lte_lc_init_and_connect();
		if (err) {
			LOG_INF("Failed to establish LTE connection: %d", err);
			return err;
		}
		LOG_INF("LTE Link Connected!");
#endif /* defined(CONFIG_LWM2M_CARRIER) */
	}
#endif /* defined(CONFIG_LTE_LINK_CONTROL) */

	return 0;
}

