/*
 * ublox_gps.c
 *
 *  Created on: Apr 25, 2017
 *      Author: Waseemh
 *      Author: MariusSR 2020
 * 		Author: Eivinhj 2021
 */
#include <zephyr.h>

#include "ublox_gps.h"
#include "../time/time_conversions.h"
#include "../drivers/cSLIM_rs232.h"
#include "../time/time_manager.h"
#include "../devices/display.h"
#include "../tasks/display_task.h"

#include <logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
LOG_MODULE_REGISTER(cSLIM_UBLOX_GPS, LOG_LEVEL_INF);

/*
 * Zephyr devices
 */

#define GPS_ENABLE_NODE	DT_ALIAS(gpsenable)
#define GPS_TIMEPULSE_NODE	DT_ALIAS(gpstimepulse)


#if DT_NODE_HAS_STATUS(GPS_ENABLE_NODE, okay) && DT_NODE_HAS_PROP(GPS_ENABLE_NODE, gpios)
#define GPS_ENABLE_GPIO_LABEL	DT_GPIO_LABEL(GPS_ENABLE_NODE, gpios)
#define GPS_ENABLE_GPIO_PIN	DT_GPIO_PIN(GPS_ENABLE_NODE, gpios)
#define GPS_ENABLE_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(GPS_ENABLE_NODE, gpios))
#endif




#if DT_NODE_HAS_STATUS(GPS_TIMEPULSE_NODE, okay)
#define GPS_TIMEPULSE_GPIO_LABEL	DT_GPIO_LABEL(GPS_TIMEPULSE_NODE, gpios)
#define GPS_TIMEPULSE_GPIO_PIN	DT_GPIO_PIN(GPS_TIMEPULSE_NODE, gpios)
#define GPS_TIMEPULSE_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(GPS_TIMEPULSE_NODE, gpios))
#else
#error "Unsupported board: BTN2 devicetree alias is not defined"
#define GPS_TIMEPULSE_GPIO_LABEL	""
#define GPS_TIMEPULSE_GPIO_PIN	0
#define GPS_TIMEPULSE_GPIO_FLAGS	0
#endif

static struct gpio_callback gps_timepulse_cb_data;

const struct device *gps_enable;
const struct device *gps_timepulse;


/* End of Zephyr devices*/

/*
 * private variables
 */

static ack_data_t  latest_ack_data;

static bool new_sat_data_available               = false;
static bool new_nav_data_available               = false;
static bool new_nav_data_available_no_fix_or_acc_requirement     = false;
static bool new_tim_data_available               = false;
static bool new_tim_data_available_high_accuracy = false;
static bool new_ack_data_available               = false;



/*
 * private function declarations
 */
static bool			send_cmd_rx_ack(uint8_t const *cmd, uint8_t size_cmd);

static void         send_cmd_no_validation( uint8_t const *cmd, uint8_t size_cmd );
static bool 		port_config( void );
static bool			disable_sbas( void );
static bool			config_low_power( void );
static bool			enter_low_power( void );
#ifndef USE_POWER_SAVING
static bool         enter_continuous_mode( void );
#endif 
static bool			poll_psm( void );
//static uint8_t 		receiver_nav_status( void );
static uint16_t		fletcher16( uint8_t const *data, uint8_t offset, size_t bytes );
static uint8_t      read_and_print_hex_from_spi( void );
static void         print_char_line_from_spi(uint8_t length);
bool 				gps_disable_pps( void );
static void         send_wakeup_msg( void );
static bool         poll_cfg_config(uint8_t const *cmd, uint8_t size_cmd);
static bool         poll_PMS_confg( void );
static bool         poll_PRT_confg( void );
static void         parse_nav_pvt_message(uint8_t data[], nav_data_t* nav_data);
//static void         message_publish_config( void );
ack_data_t          get_latest_ack_data( void );
static bool         spin_for_UBX_sync_byte( uint8_t retries );
static void         calculate_pms_wakeup_interval_msg( uint8_t* output_msg, uint16_t wakeup_interval_in_sec );


/*
 * private functions
 */
static bool send_cmd_rx_ack(uint8_t const *cmd, uint8_t size_cmd) {
    int 		outer_loop_var = 0;
    int 		inner_loop_var = 0;
    int			retry = 0;
    uint8_t		reply = 0;
    uint8_t     payload_length = 10;
    bool 		ret_flag   = false;
    bool 		break_flag = false;

    uint8_t ubx_class = cmd[2];
    uint8_t ubx_id    = cmd[3];

    //send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    for(outer_loop_var = 0; outer_loop_var < 50; ++outer_loop_var){
        for(inner_loop_var = 0; inner_loop_var < size_cmd; ++inner_loop_var){
            cSLIM_spi_read_write_byte( cmd[inner_loop_var], NULL);
        }
        
        retry = 0;
        while(1) {
            reply = cSLIM_spi_read_byte();
            if( reply == 0xB5 ){
                #ifdef DEBUG_GNSS
                //sprintf((char *)rs232_tx_buf_gnss,"\tDBG: send_cmd_rx_ack: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 23);
                k_sleep(K_MSEC(2));
                #endif
                for(inner_loop_var = 0; inner_loop_var < 9; inner_loop_var++) {
                    if (inner_loop_var >= payload_length + HEADER_LENGTH) break;

                    reply = cSLIM_spi_read_byte();
                    #ifdef DEBUG_GNSS
                    //sprintf((char *)rs232_tx_buf_gnss," 0x%02x ",reply);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 6);
                    #endif
                    if(inner_loop_var == 2) ret_flag = (reply==0x01) ? true : false;
                    else if (inner_loop_var == 3) payload_length = reply;
                    else if(inner_loop_var == 5 && reply != ubx_class) {
                        LOG_ERR( "Wrong ubx_class: 0x%02x, should be: 0x%02x\n", reply, ubx_class);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 43);
                    }
                    else if(inner_loop_var == 6 && reply != ubx_id) {
                        LOG_ERR( "Wrong ubx_id: 0x%02x, should be: 0x%02x\n", reply, ubx_id);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 40);
                    }
                    k_sleep(K_MSEC(2));
                }
                #ifdef DEBUG_GNSS
                //sprintf((char *)rs232_tx_buf_gnss, "\tack: %d\n", ret_flag);
                    //rs232_transmit_string(rs232_tx_buf_gnss, 8);
                    k_sleep(K_MSEC(2));
                #endif
                break_flag = true;
                break;
            }
            else ++retry;

            if ( retry > RETRY ) {
                ret_flag = false;
                //sprintf((char *)rs232_tx_buf_gnss,"BREAK on retry: %2d\n\n", retry);
                //rs232_transmit_string(rs232_tx_buf_gnss, 20);
                break;
            }
            
            k_sleep(K_MSEC(2));
        }
        
        if ( break_flag == true ) break;
    }
    //LOG_INF("end of loop");
    
    if(outer_loop_var >= 50) ret_flag=false;
    cSLIM_spi_cs_release(GPS);

    //printk( "CFG 0x%02x cmd acknowledged: %s\n", ubx_id, (ret_flag ? "true " : "false"));
    //rs232_transmit_string(rs232_tx_buf_gnss, 33);
    k_sleep(K_MSEC(2));
    return ret_flag;
}


static void send_cmd_no_validation( uint8_t const *cmd, uint8_t size_cmd ) {

    cSLIM_spi_cs_select(GPS);
    for(int inner_loop_var = 0; inner_loop_var < size_cmd; ++inner_loop_var)
    {
        cSLIM_spi_read_write_byte( cmd[inner_loop_var], NULL);
    }
    cSLIM_spi_cs_release(GPS);
}


void gps_send_without_checksum(uint8_t* message, uint8_t message_length) {
    send_wakeup_msg();
    
    uint8_t ck_a, ck_b;
    gps_calculate_checksum(message, message_length, &ck_a, &ck_b);
    cSLIM_spi_cs_select(GPS);
    for(int i = 0; i < message_length;i++)
    {
            cSLIM_spi_read_write_byte(message[i], NULL);
    }

    cSLIM_spi_read_write_byte(ck_a, NULL);
    cSLIM_spi_read_write_byte(ck_b, NULL);
    cSLIM_spi_cs_release(GPS);
}


void gps_send_gnss_wakeup_hot_start( void ) {

    send_wakeup_msg();
    send_cmd_no_validation(cfg_rst_gnss_enable_hot_start, (sizeof(cfg_rst_gnss_enable_hot_start)/sizeof(uint8_t)));
    
    k_sleep(K_MSEC(531));
}

void gps_send_gnss_wakeup_warm_start( void ) {

    send_wakeup_msg();
    send_cmd_no_validation(cfg_rst_gnss_enable_warm_start, (sizeof(cfg_rst_gnss_enable_warm_start)/sizeof(uint8_t)));
    k_sleep(K_MSEC(531));
}
void gps_send_gnss_wakeup_cold_start( void ) {

    send_wakeup_msg();
    send_cmd_no_validation(cfg_rst_gnss_enable_cold_start, (sizeof(cfg_rst_gnss_enable_cold_start)/sizeof(uint8_t)));
    k_sleep(K_MSEC(531));
}

void gps_send_gnss_shutdown( void ) {

   send_cmd_no_validation(cfg_rst_gnss_disable, (sizeof(cfg_rst_gnss_disable)/sizeof(uint8_t)));
}

static void send_wakeup_msg( void ) {
    //    gps_int_pin_set();
    cSLIM_spi_cs_select(GPS);
    cSLIM_spi_read_write_byte(0xFF, NULL);
    cSLIM_spi_cs_release(GPS);

    //    gps_int_pin_clear();
}

static bool poll_cfg_config(uint8_t const *cmd, uint8_t size_cmd) {
    int 		outer_loop_var=0;
    int 		inner_loop_var=0;
    int			retry=0;
    uint8_t		reply=0;
    uint8_t     payload_length = 10;
    bool 		ret_flag=true;
    bool 		break_flag=false;

    uint8_t ubx_class = cmd[2];
    uint8_t ubx_id    = cmd[3];

    //send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    for(outer_loop_var=0;outer_loop_var<50;outer_loop_var++){
        for(inner_loop_var=0;inner_loop_var<size_cmd;inner_loop_var++){
            cSLIM_spi_read_write_byte(cmd[inner_loop_var], NULL);
        }
        retry=0;
        while(1) {
            reply = cSLIM_spi_read_byte();
            if( reply == 0xB5 ){
                // UBX header
                for(inner_loop_var = 0; inner_loop_var < HEADER_LENGTH; inner_loop_var++){
                    reply = read_and_print_hex_from_spi();
                    if (inner_loop_var == 3) payload_length = reply;
                    else if(inner_loop_var == 5 && reply != ubx_class) {
                        printk( "\nERROR: wrong ubx_class: 0x%02x, should be: 0x%02x\n", reply, ubx_class);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 43);
                        ret_flag=false;
                    }
                    else if(inner_loop_var == 6 && reply != ubx_id) {
                        printk( "\nERROR: wrong ubx_id: 0x%02x, should be: 0x%02x\n", reply, ubx_id);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 40);
                        ret_flag=false;
                    }
                    k_sleep(K_MSEC(2));
                }

                // Payload
                printk("\nPayload:\t");
                //rs232_transmit_string(rs232_tx_buf_gnss, 10);
                for(inner_loop_var=0; inner_loop_var < payload_length; ++inner_loop_var) {
                    read_and_print_hex_from_spi();
                }
                printk( "\n");
                //rs232_transmit_string(rs232_tx_buf_gnss, 1);

                // Checksum. Not implemented any integrity validation, but necessary to read the whole data frame
                cSLIM_spi_dummy_read_n_byte(2);

                break_flag = true;
                break;
            }
            else ++retry;

            if ( retry > RETRY ) {
                ret_flag = false;
                break;
            }
            k_sleep(K_MSEC(2));
        }
        if ( break_flag == true ) break;
    }
    if(outer_loop_var>=50) ret_flag=false;
    cSLIM_spi_cs_release(GPS);
    k_sleep(K_MSEC(2));

    printk( "CFG 0x%02x cmd acknowledged: %s\n", ubx_id, (ret_flag ? "true " : "false"));
    //rs232_transmit_string(rs232_tx_buf_gnss, 33);
    return ret_flag;
}


static bool poll_PMS_confg( void ) {
    return poll_cfg_config(cfg_pms_poll, (sizeof(cfg_pms_poll)/sizeof(uint8_t)));
}


static bool poll_PRT_confg( void ) {
    return poll_cfg_config(cfg_prt_poll, (sizeof(cfg_prt_poll)/sizeof(uint8_t)));
}


static void print_char_line_from_spi(uint8_t length) {
    uint8_t reply;
    for(int i=0; i < length; ++i){
        reply = cSLIM_spi_read_byte();
        if (reply != 0) {
            //printk( "%c", reply);
            //rs232_transmit_string(rs232_tx_buf_gnss, 1);
            printk("%c", reply);
        }
        k_sleep(K_MSEC(2));
    }
}


static uint8_t read_and_print_hex_from_spi( void ) {
    uint8_t reply;
    reply = cSLIM_spi_read_byte();
    printk("%c", reply);
    return reply;
}



static uint16_t fletcher16( uint8_t const *data, uint8_t offset, size_t size ) {
	 uint16_t crc_a = 0;
	 uint16_t crc_b = 0;
	data += offset;

    if (size > 0) {
        do {
            crc_a += *data++;
            crc_b += crc_a;
        } while (--size);
        crc_a &= 0xff;
        crc_b &= 0xff;
    }
    return (crc_a | (crc_b << 8));
}

static bool port_config( void ) {
    bool flag_spi=false;

    while ( !send_cmd_rx_ack(ubx_cfg_cfg_clear_ioPort, (sizeof(ubx_cfg_cfg_clear_ioPort)/sizeof(uint8_t))) )
    {
        ;
    }

    #ifdef USE_TX_READY_INT
        flag_spi = send_cmd_rx_ack(cfg_prt_spi_msg_ready_interrupt_extended_timeout_8_byte_threshold, (sizeof(cfg_prt_spi_msg_ready_interrupt_extended_timeout_8_byte_threshold)/sizeof(uint8_t)));    //spi port
    #else
        flag_spi = send_cmd_rx_ack(cfg_prt_spi, (sizeof(cfg_prt_spi)/sizeof(uint8_t)));    // spi port
    #endif

    while ( !send_cmd_rx_ack(ubx_cfg_cfg_save_ioPort, (sizeof(ubx_cfg_cfg_save_ioPort)/sizeof(uint8_t))) );

    return (bool)(flag_spi);
}

/*
static void message_publish_config( void ) {
    #ifdef PUBLISH_NAV_PVT_MSG
    while ( !send_cmd_rx_ack(cfg_msg_nav_pvt_over_cSLIM_spi_enable, (sizeof(cfg_msg_nav_pvt_over_cSLIM_spi_enable)/sizeof(uint8_t))) );
    #else
    while ( !send_cmd_rx_ack(cfg_msg_nav_pvt_over_cSLIM_spi_disable, (sizeof(cfg_msg_nav_pvt_over_cSLIM_spi_disable)/sizeof(uint8_t))) );
    #endif

    #ifdef PUBLISH_TIM_TP_MSG
    while ( !send_cmd_rx_ack(cfg_msg_tim_tp_over_cSLIM_spi_enable, (sizeof(cfg_msg_tim_tp_over_cSLIM_spi_enable)/sizeof(uint8_t))) );
    #else
    while ( !send_cmd_rx_ack(cfg_msg_tim_tp_over_cSLIM_spi_disable, (sizeof(cfg_msg_tim_tp_over_cSLIM_spi_disable)/sizeof(uint8_t))) );
    #endif

    #ifdef PUBLISH_TIM_TM2_MSG
    while ( !send_cmd_rx_ack(cfg_msg_tim_tm2_over_cSLIM_spi_enable, (sizeof(cfg_msg_tim_tm2_over_cSLIM_spi_enable)/sizeof(uint8_t))) );
    #else
    while ( !send_cmd_rx_ack(cfg_msg_tim_tm2_over_cSLIM_spi_disable, (sizeof(cfg_msg_tim_tm2_over_cSLIM_spi_disable)/sizeof(uint8_t))) );
    #endif

    while ( !send_cmd_rx_ack(ubx_cfg_cfg_save_msg, (sizeof(ubx_cfg_cfg_save_msg)/sizeof(uint8_t))) );
}
*/
/*
static uint8_t receiver_nav_status( void ) {
	uint8_t reply = 0;
	int 	inner_loop_var = 0;
	int		outer_loop_var = 0;
	int 	retry = 0;
	uint8_t	fix   = 0;
	bool	flag_success = false;

    send_wakeup_msg();

	cSLIM_spi_cs_release(GPS);
//	k_sleep(K_MSEC(2));
	cSLIM_spi_cs_select(GPS);
	cSLIM_spi_read_write_byte( 0xFF, NULL);
	for(outer_loop_var=0; outer_loop_var < 50; ++outer_loop_var){
		for(inner_loop_var=0; inner_loop_var < (sizeof(nav_pvt_gps_data)/sizeof(uint8_t)); ++inner_loop_var){
			cSLIM_spi_read_write_byte(nav_pvt_gps_data[inner_loop_var], NULL);
		}
		retry = 0;

		while(1){
			reply = cSLIM_spi_read_byte();
			if(reply==0xB5){
				for(inner_loop_var=1; inner_loop_var < (88+1); ++inner_loop_var){
					reply = cSLIM_spi_read_byte();
					if(inner_loop_var-6 == 11){
						fix = (reply<<4);
					}
					if(inner_loop_var-6==20){
						fix |= (0x0F & reply);
					}
					k_sleep(K_MSEC(2));
				}
				flag_success = true;
				break;
			}
			else ++retry;

			if(retry>RETRY) break;

			k_sleep(K_MSEC(2));
		}
		if (flag_success == true) break;
	}

	if (outer_loop_var >= 50) fix = 0xFF;
	cSLIM_spi_cs_release(GPS);
	return fix;
}
*/

static bool config_low_power( void ) {
    #ifdef DEBUG_GNSS
    //sprintf((char *)rs232_tx_buf_gnss,"\tDBG: config_low_power:\t");
    //rs232_transmit_string(rs232_tx_buf_gnss, 24);
    #endif

    #ifdef USE_PMS
        #ifdef USE_PMS_BALANCED
            return (bool) send_cmd_rx_ack(cfg_pms_balanced, (sizeof(cfg_pms_balanced)/sizeof(uint8_t)));
        #else
            return (bool) send_cmd_rx_ack(cfg_pms_interval_3600s_6s_on_time, (sizeof(cfg_pms_interval_3600s_6s_on_time)/sizeof(uint8_t)));
        #endif
    #else
        return (bool) send_cmd_rx_ack(cfg_pms_full_power, (sizeof(cfg_pms_full_power)/sizeof(uint8_t)));
    #endif
}


static bool enter_low_power( void ) {
    return (bool) send_cmd_rx_ack(cfg_rxm_psm_mode, (sizeof(cfg_rxm_psm_mode)/sizeof(uint8_t)));
}


// TODO: Not tested
#if !USE_POWER_SAVING
static bool enter_continuous_mode( void ) {
    #if  USE_FULL_POWER
        return (bool) send_cmd_rx_ack(cfg_pms_full_power, (sizeof(cfg_pms_full_power)/sizeof(uint8_t)));
    #else
        return (bool) send_cmd_rx_ack(cfg_rxm_psm_continuous_mode, (sizeof(cfg_rxm_psm_continuous_mode)/sizeof(uint8_t)));
    #endif
}
#endif

static bool disable_sbas( void ) {
#ifdef DEBUG_GNSS
    //sprintf((char *)rs232_tx_buf_gnss,"\tDBG: cfg_sbas_disable:\t");
    //rs232_transmit_string(rs232_tx_buf_gnss, 24);
    k_sleep(K_MSEC(2));
#endif
    return (bool) send_cmd_rx_ack(cfg_sbas_disable, (sizeof(cfg_sbas_disable)/sizeof(uint8_t)));
}


static bool poll_psm( void ) {
	bool 	flag = false;
	uint8_t reply = 0;
	int 	inner_loop_var = 0;
	int		outer_loop_var = 0;
	int 	retry = 0;
	bool	flag_success = false;

//    const unsigned char rs232_tx_buf_gnss[64];

    //send_wakeup_msg();

	cSLIM_spi_cs_release(GPS);
	k_sleep(K_MSEC(2));
	cSLIM_spi_cs_select(GPS);
	cSLIM_spi_read_write_byte(0xFF, NULL);

	for(outer_loop_var = 0; outer_loop_var < 50; ++outer_loop_var){

	    // Send request
	    for(inner_loop_var = 0; inner_loop_var < (sizeof(cfg_rxm_poll_psm)/sizeof(uint8_t)); ++inner_loop_var)
        {
			cSLIM_spi_read_write_byte(cfg_rxm_poll_psm[inner_loop_var], NULL);
		}
		retry = 0;
		while(1){
			reply = cSLIM_spi_read_byte();
			if(reply == 0xB5){
                for(inner_loop_var=0; inner_loop_var < 9; inner_loop_var++){
					reply = cSLIM_spi_read_byte();
					k_sleep(K_MSEC(5));

                    // Set flag if system is in lpMode Power Save Mode (0x01)
					if(inner_loop_var == HEADER_LENGTH + 1) flag = (reply == 0x01) ? true : false;
					k_sleep(K_MSEC(2));
				}
				flag_success = true;
				break;
			}
			else ++retry;

			if(retry > RETRY) break;
			k_sleep(K_MSEC(2));
		}
		if(flag_success == true) break;
	}
	if(outer_loop_var >= 50) flag = false;
	cSLIM_spi_cs_release(GPS);
	return flag;
}


void update_sat_data(const ubx_msg_t* msg, sat_data_t* sat_data) {
    sat_data->numSV   = msg->payload[5];
    sat_data->cno_min = 0;
    sat_data->cno_max = 0;

    uint8_t SV_cno;
    for (int SV = 0; SV < sat_data->numSV; ++SV) {
        SV_cno = msg->payload[10+12*SV];
        if ((SV_cno < sat_data->cno_min) | (sat_data->cno_min == 0)) sat_data->cno_min = SV_cno;
        if (SV_cno > sat_data->cno_max) sat_data->cno_max = SV_cno;
    }
    new_sat_data_available = true;
    //sprintf((char *)rs232_tx_buf_gnss, "\t New SAT data available\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 25);
    //k_sleep(K_MSEC(2));
}

void update_nav_data(ubx_msg_t* msg, nav_data_t* nav_data) {
    parse_nav_pvt_message(msg->payload, nav_data);

    nav_data->recv_time_local = msg->recv_time_local;
    nav_data->BURTC_subsec_tick = msg->BURTC_subsec_tick;

//    if (nav_data->valid) {
//       uint32_t recv_valid  = BURTC_CounterGet();
//        uint32_t dumme = recv_valid - 0;
//    }

    //sprintf((char *)rs232_tx_buf_gnss, "NAV msg data. fix: %2d, numSV: %2d, valid: %1d, validDate: %1d, validTime: %1d, gpsTime: %10d, long: %10d, lat: %10d, tAcc: %10d\n", 
    //    nav_data->fix, nav_data->numSV, nav_data->valid, nav_data->valid_date, nav_data->valid_time, nav_data->unix_tstamp, nav_data->longitude, nav_data->latitude, nav_data->tAcc);


    //rs232_transmit_string(rs232_tx_buf_gnss, 145);


    if (nav_data->valid){
        LOG_INF("New valid NAV data available\n");
        new_nav_data_available = true;
    }
    else new_nav_data_available = false;

    new_nav_data_available_no_fix_or_acc_requirement = true;
}


void parse_tim_data(ubx_msg_t* msg, time_data_t* tim_data) {
    //uint32_t prev_tow_ms = tim_data->tow_ms;

    uint32_t tow_ms = 0;
    tow_ms |= (msg->payload[3] << 24);
    tow_ms |= (msg->payload[2] << 16);
    tow_ms |= (msg->payload[1] << 8);
    tow_ms |= msg->payload[0];
    tim_data->tow_ms = tow_ms;

    uint32_t tow_sub_ms = 0;
    tow_sub_ms |= (msg->payload[7] << 24);
    tow_sub_ms |= (msg->payload[6] << 16);
    tow_sub_ms |= (msg->payload[5] << 8);
    tow_sub_ms |= msg->payload[4];
    tim_data->tow_sub_ms = tow_sub_ms;

    uint32_t quantisation_error = 0;
    quantisation_error |= (msg->payload[11] << 24);
    quantisation_error |= (msg->payload[10] << 16);
    quantisation_error |= (msg->payload[9] << 8);
    quantisation_error |= msg->payload[8];
    tim_data->quantisation_error = quantisation_error;

    uint16_t week = 0;
    week |= (msg->payload[13] << 8);
    week |= msg->payload[12];
    tim_data->week = week;

    tim_data->flags    = msg->payload[14];
    tim_data->ref_info = msg->payload[15];

}


void update_ack_data(const ubx_msg_t* msg, ack_data_t* ack_data, const bool is_accepted) {
    ack_data->valid = true;
    ack_data->is_accepted = is_accepted;
    ack_data->class = msg->payload[0];
    ack_data->id = msg->payload[1];

    new_ack_data_available = true;
    k_sleep(K_MSEC(2));
}


static void parse_nav_pvt_message( uint8_t data[],  nav_data_t* nav_data ) {
    uint32_t    unix_tstamp = 0;
    uint32_t    tAcc = 0;
    uint32_t    nano = 0;
    uint16_t    year = 0;
    uint32_t    longitude = 0;
    uint32_t    latitude = 0;
    uint32_t    height = 0;
//    uint8_t	    offset = 6;
    uint16_t    pdop = 0;

    //extract GPS TimeStamp
    unix_tstamp |= (data[3]<<24);
    unix_tstamp |= (data[2]<<16);
    unix_tstamp |= (data[1]<<8);
    unix_tstamp |= data[0];
    nav_data->unix_tstamp = (uint32_t)(unix_tstamp);

    // extract date
    year  = data[5];
    year  = year<<8;
    year |= data[4];
    nav_data->year  = year;
    nav_data->month = data[6];
    nav_data->day   = data[7];

    // extract time
    nav_data->hour = data[8];
    nav_data->min  = data[9];
    nav_data->sec  = data[10];

    // extract timing accuracy
    nav_data->t_flags = data[11];
    tAcc |= (data[15]<<24);
    tAcc |= (data[14]<<16);
    tAcc |= (data[13]<<8);
    tAcc |= data[12];
    nav_data->tAcc = (uint32_t)(tAcc);
    nano |= (data[19]<<24);
    nano |= (data[18]<<16);
    nano |= (data[17]<<8);
    nano |= data[16];
    nav_data->nano  = (int32_t)(nano);
    nav_data->fix   = data[20];
    nav_data->numSV = data[23];
    pdop |= (data[77]<<8);
    pdop |= data[76];
    nav_data->pDOP = (uint16_t)(pdop);

    //extract longitude
    longitude  = 0;
    longitude |= (data[27]<<24);
    longitude |= (data[26]<<16);
    longitude |= (data[25]<<8);
    longitude |= data[24];
    nav_data->longitude = (uint32_t)(longitude);

    //extract latitude
    latitude  = 0;
    latitude |= (data[31]<<24);
    latitude |= (data[30]<<16);
    latitude |= (data[29]<<8);
    latitude |= (data[28]);
    nav_data->latitude = (uint32_t)(latitude);

    //extract height over mean sea level
    height  = 0;
    height |= (data[39]<<24);
    height |= (data[38]<<16);
    height |= (data[37]<<8);
    height |= data[36];
    nav_data->height = (uint32_t)(height);

    // Flags
    nav_data->valid_date     = (data[11] & (1 << 0));
    nav_data->valid_time     = (data[11] & (1 << 1));
    nav_data->fully_resolved = (data[11] & (1 << 2));

    nav_data->gnss_fix_ok    = (data[21] & (1 << 0));

    nav_data->confirmed_date = (data[22] & (1 << 6));
    nav_data->confirmed_time = (data[22] & (1 << 7));

//    nav_data->valid = ( ( data[20] == TWO_DIM_FIX || data[20]== THREE_DIM_FIX ) && nav_data->valid_date && nav_data->valid_time) ;
    nav_data->valid = ( ( data[20] == TWO_DIM_FIX || data[20]== THREE_DIM_FIX ) && nav_data->valid_date && nav_data->valid_time && nav_data->gnss_fix_ok) ;

    unsigned char display_buf[40];
    static int print_fix   = 0;
    static int print_numSV = 0;
//    static times_since_update_cnt = 11;
//    if ( (nav_data->fix != print_fix) || (nav_data->numSV != print_numSV) || (times_since_update_cnt++ > 10) ) {
    if ( (nav_data->fix != print_fix) || (nav_data->numSV != print_numSV) ) {
//        times_since_update_cnt = 0;
        print_fix = nav_data->fix;
        print_numSV = nav_data->numSV;
//        //sprintf((char *)display_buf, "Fix: %1d, nSV:%2d", nav_data->fix, nav_data->numSV);
//        display_put_string(3, 9*12, display_buf, font_small);

        if ( nav_data->fix == NO_FIX)             sprintf((char *)display_buf, "NO-fix %2d",  nav_data->numSV);
        else if ( nav_data->fix == TWO_DIM_FIX)   sprintf((char *)display_buf, "2D-fix %2d",  nav_data->numSV);
        else if ( nav_data->fix == THREE_DIM_FIX) sprintf((char *)display_buf, "3D-fix %2d",  nav_data->numSV);
        else                                     sprintf((char *)display_buf, " %1d-fix %2d", nav_data->fix, nav_data->numSV);
        display_put_string(6 + 3*12 + 2, 4*12+5, display_buf, font_medium);
        //display_update();
        request_display_update();
    }

}


/*
* public functions
*/
bool gps_init(gpio_callback_handler_t gpio_timepulse_cb) {
    LOG_INF("Enter init");
    unsigned char           display_buf[40];

	uint8_t                 prev_numSV = 255;
	bool					flag_port = false;
	bool					flag_init = false;
	bool					flag_config_lp = false;
	bool					flag_sbas = false;
	bool					flag_enter_lp = false;
	bool					flag_select_gps = false;
	bool					flag_config_tp = false;
//	nav_data_t				start_fix;
    sat_data_t              sat_data;
	uint8_t ret = 0;
//	cSLIM_spi_init();
    //cSLIM_CS_init(GPS);

    //Get GPIO device bindings
	gps_enable = device_get_binding(GPS_ENABLE_GPIO_LABEL);

		if (gps_enable == NULL) 
		{
			printk("Didn't find device %s\n", GPS_ENABLE_GPIO_LABEL);
			return 0;
		}
		ret = gpio_pin_configure(gps_enable, GPS_ENABLE_GPIO_PIN, GPS_ENABLE_GPIO_FLAGS);
		if (ret != 0) 
		{
			printk("Error %d: failed to configure device %s pin %d\n",
					ret, GPS_ENABLE_GPIO_LABEL, GPS_ENABLE_GPIO_PIN);
			return ret;
		}

	gps_timepulse = device_get_binding(GPS_TIMEPULSE_GPIO_LABEL);
	if (gps_timepulse == NULL) 
	{
		printk("Error: didn't find %s device\n", GPS_TIMEPULSE_GPIO_LABEL);
		ret = 1;
		return ret;
	}
	ret = gpio_pin_configure(gps_timepulse, GPS_TIMEPULSE_GPIO_PIN, GPS_TIMEPULSE_GPIO_FLAGS);
	if (ret != 0) 
	{
		printk("Error %d: failed to configure %s pin %d\n", ret, GPS_TIMEPULSE_GPIO_LABEL, GPS_TIMEPULSE_GPIO_PIN);
		return ret;
	}
	ret = gpio_pin_interrupt_configure(gps_timepulse,
			GPS_TIMEPULSE_GPIO_PIN,
			GPIO_INT_EDGE_RISING); 
	if (ret != 0) 
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, GPS_TIMEPULSE_GPIO_LABEL, GPS_TIMEPULSE_GPIO_PIN);
		ret = 1;
		return ret;
	}


    //Enable GPS module
	gpio_pin_set(gps_enable, GPS_ENABLE_GPIO_PIN, 0);
    //Give GPS module time to wake up
    k_sleep(K_MSEC(500));
    
    //Read GPS 
    #ifdef CLEAR_SETTINGS_ON_BOOT
    /// Reset non-volatile configurations to default, and load the configurations into the current config
    //TEST
    while( !send_cmd_rx_ack(ubx_cfg_cfg_clear_and_load_nav_and_rxm, (sizeof(ubx_cfg_cfg_clear_and_load_nav_and_rxm)/sizeof(uint8_t))) )
    {
        ;
    }

    #endif
//    while( !send_cmd_rx_ack(ubx_cfg_cfg_clear_and_load_all, (sizeof(ubx_cfg_cfg_clear_and_load_all)/sizeof(uint8_t))) );

    /// Configure ports
	flag_port = port_config();
	flag_select_gps = send_cmd_rx_ack(cfg_gnss, (sizeof(cfg_gnss)/sizeof(uint8_t)));
    #ifdef USE_TIME_PULSE_SIGNAL
    //flag_config_tp = send_cmd_rx_ack(cfg_tp5_pps_without_fix,  (sizeof(cfg_tp5_pps_without_fix)/sizeof(uint8_t)));
    //flag_config_tp = send_cmd_rx_ack(cfg_tp5_pps_only_with_fix,  (sizeof(cfg_tp5_pps_only_with_fix)/sizeof(uint8_t)));
    flag_config_tp = send_cmd_rx_ack(cfg_tp5_pps_only_with_fix_new,  (sizeof(cfg_tp5_pps_only_with_fix_new)/sizeof(uint8_t)));
    #else
    flag_config_tp  = send_cmd_rx_ack(cfg_tp5_disable,  (sizeof(cfg_tp5_disable)/sizeof(uint8_t)));
    #endif

    //bool flag_config_rate  = send_cmd_rx_ack(ubx_cfg_rate_1s,  (sizeof(ubx_cfg_rate_1s)/sizeof(uint8_t)));
    bool flag_config_rate  = send_cmd_rx_ack(ubx_cfg_rate_1s_gps_time,  (sizeof(ubx_cfg_rate_1s_gps_time)/sizeof(uint8_t)));  //UTC time takes up to 12.5 min to aquire from gps satellites, use pure gps time when testing

    // Configure stationary dynamic model
//    send_cmd_rx_ack(ubx_nav5_stationary_model,  (sizeof(ubx_nav5_stationary_model)/sizeof(uint8_t)));
//    send_cmd_rx_ack(ubx_nav5_stationary_model_with_timemask,  (sizeof(ubx_nav5_stationary_model_with_timemask)/sizeof(uint8_t)));

    send_cmd_rx_ack(cfg_pms_full_power, (sizeof(cfg_pms_full_power)/sizeof(uint8_t)));

    while ( !send_cmd_rx_ack(ubx_cfg_cfg_save_nav_and_rxm, (sizeof(ubx_cfg_cfg_save_nav_and_rxm)/sizeof(uint8_t))) );

    gps_get_nav5();

//	gps_get_firmware_version();

    /// NAV SAT
    /// Wait for fix
    LOG_INF("GPS: waiting for fix\n");

    
    gps_poll_sat_status();
    while(1) {
        sat_data = gps_get_sat_data();
        if (sat_data.valid) {
            if (sat_data.numSV != prev_numSV) {
                prev_numSV = sat_data.numSV;
              
                LOG_INF( "\tGPS:\tnumSv: %3d\tMax CNO: %3d\tMin CNO: %3d\tnumSv_over_40dBHz: %3d", sat_data.numSV, sat_data.cno_max, sat_data.cno_min, sat_data.numSV_over_40_dBHz);

                // Draw stats table on display
                sprintf((char *) display_buf, "numSV     CNO"); ;
                display_put_string(1, 5 * 12 + 5, display_buf, font_small);
                sprintf((char *) display_buf, "max   min");
                display_put_string(65, 6 * 12 + 4, display_buf, font_small);
                sprintf((char *) display_buf, "%2d/%d  %3d   %3d", sat_data.numSV_over_40_dBHz, sat_data.numSV, sat_data.cno_max, sat_data.cno_min);
                //display_put_string(64-(6*6), 5*12+4, display_buf, font_small);
                display_put_string(20, 7*12+4, display_buf, font_small);
                display_draw_horisontal_whole_line(6*12);
                display_draw_horisontal_whole_line(5*12 + 4);
                display_draw_vertical_line(62, 5*12+5, 7*12+4+8*2);
                display_draw_vertical_line(98 + 0, 6*12+2, 7*12+4+8*2);
                //display_update();
                request_display_update();
            }
            if(sat_data.numSV_over_40_dBHz >= 4){
                flag_init = true;
                break;
            }
        } 
//        start_fix = gps_get_nav_data();
        if(!flag_init){
            gps_poll_sat_status();
            k_sleep(K_SECONDS(1));
        }
    }
    
   flag_init = true;
    
    flag_sbas = disable_sbas(); //For best timepulse performance it is recommended to disable sbas, also reccomended for power saving

    #if USE_POWER_SAVING
    	/// Configure and enter low power
        flag_config_lp = config_low_power();
        flag_enter_lp = enter_low_power();

        gps_print_PMS_config();

        // poll PSM and exit if OK
        for(int i = 0; i < 60; ++i){
            if(poll_psm()){
                LOG_INF("\nGPS: Power saving mode active\n");
                
                break;
            }
            enter_low_power();
            k_sleep(K_MSEC(9));
            if (i == 59) {
                LOG_INF("\nGPS: Power saving mode OFF!\n");
            }
        }
    #else
        enter_continuous_mode();
        //poll PSM and exit if OK
        for(int i = 0; i < 60; ++i){
            if(!poll_psm()){
                LOG_INF("\nGPS: Power saving mode OFF\n");
                break;
            }
            k_sleep(K_MSEC(9));
            if (i == 59) {
               LOG_INF("\nGPS: Power saving mode ON!\n");
            }
        }
    #endif

    /// Save configuration to non-volatile memory
    //gps_print_PMS_config();
     while ( !send_cmd_rx_ack(ubx_cfg_cfg_save_nav_and_rxm, (sizeof(ubx_cfg_cfg_save_nav_and_rxm)/sizeof(uint8_t))));
    gps_print_PMS_config();
    

    //Add timepulse callback
	gpio_init_callback(&gps_timepulse_cb_data, gpio_timepulse_cb, BIT(GPS_TIMEPULSE_GPIO_PIN));
	gpio_add_callback(gps_timepulse, &gps_timepulse_cb_data);

    return (bool) (flag_port==true && flag_sbas==true && flag_init==true && flag_config_tp==true && flag_select_gps==true && flag_config_rate==true);    
}


bool gps_print_PMS_config( void ) {
    LOG_INF(" poll PMS config\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 22);
    return poll_PMS_confg();
}


bool gps_print_PRT_config( void ) {
    //sprintf((char *)rs232_tx_buf_gnss, "\nGPS: poll PRT config\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 22);
    return poll_PRT_confg();
}


void gps_poll_nav_data ( void ) {
    //unsigned char rs232_tx_buf_gnss[37];
    LOG_DBG( "GPS gps_poll_nav_data called");
    //rs232_transmit_string(rs232_tx_buf_gnss, 37);

    cSLIM_spi_cs_select(GPS);
    LOG_DBG( "CS complete");
    for(int i = 0; i < (sizeof(nav_pvt_gps_data)/sizeof(uint8_t)); ++i){
        cSLIM_spi_read_write_byte(nav_pvt_gps_data[i], NULL);
    }
    LOG_DBG( "read bytes");
    cSLIM_spi_cs_release(GPS);
    LOG_DBG( "GPS gps_poll_nav_data complete");
}

void gps_poll_sat_status ( void ) {
    LOG_DBG( "GPS gps_poll_sat_status called");
    //rs232_transmit_string(rs232_tx_buf_gnss, 37);
    //send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    for (int i = 0; i < (sizeof(nav_sat_gps_data)/sizeof(uint8_t)); ++i){
        cSLIM_spi_write_byte(nav_sat_gps_data[i]);
    }
    cSLIM_spi_cs_release(GPS);
    k_sleep(K_MSEC(2));
}


void gps_poll_tim_status ( void ) {
    LOG_DBG( "gps_poll_tim_status: called\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 38);

    send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    LOG_DBG( "gps_poll_tim_status: cs selected\n");
    for(int i = 0; i < (sizeof(ubx_tim_tp_poll)/sizeof(uint8_t)); ++i){
        cSLIM_spi_write_byte(ubx_tim_tp_poll[i]);
    }
    LOG_DBG( "gps_poll_tim_status: bytes written\n");
    cSLIM_spi_cs_release(GPS);
    k_sleep(K_MSEC(2));
    LOG_DBG( "gps_poll_tim_status: complete\n");
}


bool gps_poll_tim_status_polling_based (ubx_msg_t*   recv_msg) {
    LOG_DBG("\nGPS gps_poll_tim_status_polling_based called\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 51);

    unsigned char spi_buffer[10]; // TODO: used for debugging

    cSLIM_spi_cs_select(GPS);
    for(int i = 0; i < (sizeof(ubx_tim_tp_poll)/sizeof(uint8_t)); ++i){
//        spi_write_byte(ubx_tim_tp_poll[i]);
        cSLIM_spi_read_write_byte(ubx_tim_tp_poll[i], &(spi_buffer[i]));
    }
  


  LOG_DBG("gps_poll_tim_status_polling_based spi content: ");
    //rs232_transmit_string(rs232_tx_buf_gnss, 53);
    for(int i = 0; i < (sizeof(nav_pvt_gps_data)/sizeof(uint8_t)); ++i){
        LOG_DBG(" 0x%02x ", spi_buffer[i]);
       
    }


    //unsigned char rs232_tx_buf_gnss[60];

//    int 		outer_loop_var = 0;
//    int 		inner_loop_var = 0;
    uint8_t		reply = 0;
//  uint8_t		cmd   = 0;
    bool        flag_success = false;
    uint32_t    recv_b5_on_burtc_tick;


    LOG_INF("gps_read_spi_data: start");
  

    flag_success = spin_for_UBX_sync_byte(20);
    recv_b5_on_burtc_tick = k_cycle_get_32();

    if (!flag_success) 
    {
            LOG_DBG( "read_cSLIM_spi_data() no response.");
    }
    else {
        recv_msg->recv_time_local = time_manager_getLocalUnixTime();
        recv_msg->BURTC_subsec_tick = recv_b5_on_burtc_tick;

        // Header
        recv_msg->header[0] = reply;
        recv_msg->header[1] = cSLIM_spi_read_byte();
        recv_msg->class     = cSLIM_spi_read_byte();
        recv_msg->id        = cSLIM_spi_read_byte();
        recv_msg->length    = cSLIM_spi_read_byte();
        reply              = cSLIM_spi_read_byte();       // dummy read. Implement for payload larger than 256 bytes

        LOG_INF("gps_read_spi_data: Length %d\n", recv_msg->length);

        // Payload
        for (int byte = 0; byte < recv_msg->length; ++byte) {
            reply = cSLIM_spi_read_byte();
            recv_msg->payload[byte] = reply;
        }

        // Checksum
        recv_msg->checksum[0] = cSLIM_spi_read_byte();
        recv_msg->checksum[1] = cSLIM_spi_read_byte();
    }


    cSLIM_spi_cs_release(GPS);
    return flag_success;

}


sat_data_t gps_get_sat_data ( void ) {
    sat_data_t 	sat_data;
    sat_data.valid = false;
    sat_data.cno_min = 0;
    sat_data.cno_max = 0;
    sat_data.numSV = 0;
    sat_data.numSV_over_40_dBHz = 0;

    int 		outer_loop_var = 0;
    int 		inner_loop_var = 0;
    //int 		retry = 0;
    uint8_t		reply = 0;
    uint8_t		cmd = 0;
    bool        flag_success = true;

    cSLIM_spi_cs_select(GPS);
    flag_success = spin_for_UBX_sync_byte(255);

    if( flag_success == true ) {
        // Header and 8 first bytes of the payload
        for(inner_loop_var = 0; inner_loop_var < (8 + HEADER_LENGTH); ++inner_loop_var) {
            reply = cSLIM_spi_read_byte();

            if (inner_loop_var == 2) cmd = reply;
            if ((inner_loop_var == 5 + HEADER_LENGTH) && (cmd == 53)){      // 53 == CFG == 0x35, UBX_NAV_SAT msg
                sat_data.numSV = reply;
                sat_data.valid = true;
            }
        }

        // Repeated payload based on numSV seen
        if (sat_data.valid) {
            for (outer_loop_var = 0; outer_loop_var < sat_data.numSV; outer_loop_var++) {
                for (inner_loop_var = 0; inner_loop_var < 12; inner_loop_var++) {
                    reply = cSLIM_spi_read_byte();
//                    msg_buf[inner_loop_var + 12 * outer_loop_var + 13] = reply;
                    if (inner_loop_var == 3) {
                        if ((reply < sat_data.cno_min) | (sat_data.cno_min == 0)) sat_data.cno_min = reply;
                        if (reply > sat_data.cno_max) sat_data.cno_max = reply;
                        if (reply >= 30) ++sat_data.numSV_over_40_dBHz;		//todo: husk at denne er sett til 30 og 40 lenger
                    }
                }
            }

            // Checksum. Not implemented any integrity validation, but necessary to read the whole data frame
            cSLIM_spi_dummy_read_n_byte(2);
        }
    }
    cSLIM_spi_cs_release(GPS);
    return sat_data;
}

// interrupt based
void gps_read_all_spi_data ( void ) {
    //bool read_msg = true;
/*    while ( (gpio_pin_get(gps_extint,GPS_EXTINT_GPIO_PIN) == true) ) {      // Rising edge
        read_msg = gps_read_spi_data();
        k_sleep(K_MSEC(1));            // In order to allow for the tx_ready signal to go low
    }*/
}


// TODO: gjøre static og heller lage ett interface?
bool gps_read_spi_data (ubx_msg_t*   recv_msg) {
    //unsigned char rs232_tx_buf_gnss[60];

//    int 		outer_loop_var = 0;
//    int 		inner_loop_var = 0;
    uint8_t		reply = 0;
//  uint8_t		cmd   = 0;
    bool        flag_success = false;
    uint32_t    recv_b5_on_burtc_tick;


    LOG_INF("gps_read_spi_data: start");
    cSLIM_spi_cs_select(GPS);

    flag_success = spin_for_UBX_sync_byte(20);
    recv_b5_on_burtc_tick = k_cycle_get_32();

    if (!flag_success) 
    {
            LOG_DBG( "read_cSLIM_spi_data() no response.");
    }
    else {
        recv_msg->recv_time_local = time_manager_getLocalUnixTime();
        recv_msg->BURTC_subsec_tick = recv_b5_on_burtc_tick;

        // Header
        recv_msg->header[0] = reply;
        recv_msg->header[1] = cSLIM_spi_read_byte();
        recv_msg->class     = cSLIM_spi_read_byte();
        recv_msg->id        = cSLIM_spi_read_byte();
        recv_msg->length    = cSLIM_spi_read_byte();
        reply              = cSLIM_spi_read_byte();       // dummy read. Implement for payload larger than 256 bytes

        LOG_INF("gps_read_spi_data: Length %d\n", recv_msg->length);

        // Payload
        for (int byte = 0; byte < recv_msg->length; ++byte) {
            reply = cSLIM_spi_read_byte();
            recv_msg->payload[byte] = reply;
        }

        // Checksum
        recv_msg->checksum[0] = cSLIM_spi_read_byte();
        recv_msg->checksum[1] = cSLIM_spi_read_byte();
    }


    cSLIM_spi_cs_release(GPS);
    return flag_success;
}

/*
bool is_new_nav_data_available( void ) {
    return new_nav_data_available;
}


bool is_new_nav_data_available_no_fix_or_acc_requirement( void ) {
    return new_nav_data_available_no_fix_or_acc_requirement;
}


bool is_new_sat_data_available( void ) {
    return new_sat_data_available;
}


bool is_new_tim_data_available( void ) {
    return new_tim_data_available;
}

bool is_new_accurate_tim_data_available( void ) {
    return new_tim_data_available_high_accuracy;
}
*/

/*nav_data_t get_latest_nav_data( void ) {
    new_nav_data_available = false;
    new_nav_data_available_no_fix_or_acc_requirement = false;
    return latest_nav_data;
}*/




ack_data_t get_latest_ack_data( void ) {
    new_ack_data_available = false;
    return latest_ack_data;
}


void gps_clear_tim_data( void ) {
    new_tim_data_available = false;
    new_tim_data_available_high_accuracy = false;
}


/*
void gps_on( void ) {
	gpio_pin_set(gps_enable,GPS_ENABLE_GPIO_PIN, 1);
}


void gps_off( void ) {
	gpio_pin_set(gps_enable,GPS_ENABLE_GPIO_PIN, 0);
}
 */


void gps_int_pin_set( void ) {
    #ifndef USE_TX_READY_INT
	//gpio_pin_set(gps_extint,GPS_EXTINT_GPIO_PIN, 1);
    //set_gpio(AUX_1);
    #endif
}


void gps_int_pin_clear( void ) {
    #ifndef USE_TX_READY_INT
    //gpio_pin_set(gps_extint,GPS_EXTINT_GPIO_PIN, 0);
    //clear_gpio(AUX_1);
    #endif
}


void gps_int_pin_toggle( void ) {
    #ifndef USE_TX_READY_INT
    //gpio_pin_toggle(gps_extint,GPS_EXTINT_GPIO_PIN);
    //gpio_toggle(AUX_1);
    #endif
}


bool gps_get_firmware_version( void ) {
    int 		outer_loop_var=0;
    int 		inner_loop_var=0;
    int			retry=0;
    uint8_t		reply=0;
    uint8_t     payload_length = 10;
    bool        ret_flag=true;
    bool 		break_flag=false;

    const uint8_t* cmd = ubx_mon_ver;
    uint8_t size_cmd = sizeof(ubx_mon_ver) / sizeof(uint8_t);
    uint8_t ubx_class = ubx_mon_ver[2];
    uint8_t ubx_id    = ubx_mon_ver[3];

    //const unsigned char rs232_tx_buf_gnss[64];

    //send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    //for(outer_loop_var=0; outer_loop_var<50; outer_loop_var++){
        for(inner_loop_var=0; inner_loop_var < size_cmd; inner_loop_var++){
            cSLIM_spi_read_write_byte(cmd[inner_loop_var], &reply);
            //LOG_INF("Reply: %x", reply);
        }
        retry=0;
        while(1) {
            reply = cSLIM_spi_read_byte();
            //LOG_INF("Reply: %x", reply);
            if(reply == 0xB5){
                ////sprintf((char *)rs232_tx_buf_gnss,"\nFirmware_version:\nHeader: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 27);
                printk("\nFirmware_version:\nHeader: ");

                // UBX header
                for(inner_loop_var=0; inner_loop_var < HEADER_LENGTH; inner_loop_var++){
                    reply = read_and_print_hex_from_spi();
                    if (inner_loop_var == 3) payload_length = reply;
                    else if(inner_loop_var == 5 && reply != ubx_class) {
                        //printk( "\nERROR: wrong ubx_class: 0x%02x, should be: 0x%02x\n", reply, ubx_class);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 43);
                        printk("\nERROR: wrong ubx_class: 0x%02x, should be: 0x%02x\n", reply, ubx_class);
                        ret_flag=false;
                    }
                    else if(inner_loop_var == 6 && reply != ubx_id) {
                        //printk( "\nERROR: wrong ubx_id: 0x%02x, should be: 0x%02x\n", reply, ubx_id);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 40);
                        printk("\nERROR: wrong ubx_id: 0x%02x, should be: 0x%02x\n", reply, ubx_id);
                        ret_flag=false;
                    }
                    k_sleep(K_MSEC(2));
                }

                // SW version
                ////sprintf((char *)rs232_tx_buf_gnss,"\nSW version: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 13);
                printk("\nSW version: ");
                print_char_line_from_spi(30);

                // HW version
                ////sprintf((char *)rs232_tx_buf_gnss,"\nHW version: ");
                printk("\nHW version: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 13);
                print_char_line_from_spi(10);

                // Extension
                ////sprintf((char *)rs232_tx_buf_gnss,"\nExtensions:\n");
                printk("\nExtensions:\n");
                //rs232_transmit_string(rs232_tx_buf_gnss, 13);
                for(inner_loop_var=0; inner_loop_var < ((payload_length-30)/30); ++inner_loop_var) {
                    print_char_line_from_spi(30);
                    //printk( "\n");
                    printk("\n");
                    //rs232_transmit_string(rs232_tx_buf_gnss, 1);
                }

                //printk( "\n");
                printk("\n");
                //rs232_transmit_string(rs232_tx_buf_gnss, 1);

                // Checksum. Not implemented any integrity validation, but necessary to read the whole data frame
                cSLIM_spi_dummy_read_n_byte(2);

                break_flag = true;
                break;
            }
            else ++retry;

            if(retry > RETRY){
                ret_flag = false;
                break;
            }
            k_sleep(K_MSEC(2));
        }
        
        //if(break_flag == true) break;
    //}
    if (outer_loop_var >= 50) ret_flag = false;
    cSLIM_spi_cs_release(GPS);
    k_sleep(K_MSEC(2));
    return ret_flag;
}


bool gps_get_nav5( void ) {
    LOG_DBG("gps_get_nav5 called\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 21);
    int 		outer_loop_var=0;
    int 		inner_loop_var=0;
    int			retry=0;
    uint8_t		reply=0;
    uint8_t     payload_length = 36;
    bool        ret_flag=true;
    bool 		break_flag=false;

    const uint8_t* cmd = ubx_nav5_get;
    uint8_t size_cmd = sizeof(ubx_mon_ver) / sizeof(uint8_t);
    uint8_t ubx_class = ubx_mon_ver[2];
    uint8_t ubx_id    = ubx_mon_ver[3];

    //const unsigned char rs232_tx_buf_gnss[64];

    send_wakeup_msg();

    cSLIM_spi_cs_select(GPS);
    for(outer_loop_var=0; outer_loop_var<50; outer_loop_var++){
        for(inner_loop_var=0; inner_loop_var < size_cmd; inner_loop_var++){
            cSLIM_spi_read_write_byte(cmd[inner_loop_var], NULL);
        }
        retry=0;
        while(1) {
            reply = cSLIM_spi_read_byte();
            if(reply == 0xB5){
                //sprintf((char *)rs232_tx_buf_gnss,"\nNav5 settings:\nHeader: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 24);

                // UBX header
                for(inner_loop_var=0; inner_loop_var < HEADER_LENGTH; inner_loop_var++){
                    reply = read_and_print_hex_from_spi();
                    if (inner_loop_var == 3) payload_length = reply;
                    else if(inner_loop_var == 5 && reply != ubx_class) {
                       LOG_ERR("Wrong ubx_class: 0x%02x, should be: 0x%02x\n", reply, ubx_class);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 43);
                        ret_flag=false;
                    }
                    else if(inner_loop_var == 6 && reply != ubx_id) {
                        LOG_ERR("Wrong ubx_id: 0x%02x, should be: 0x%02x\n", reply, ubx_id);
                        //rs232_transmit_string(rs232_tx_buf_gnss, 40);
                        ret_flag=false;
                    }
                    k_sleep(K_MSEC(2));
                }

                //sprintf((char *)rs232_tx_buf_gnss,"\nmask: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 8);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();


                //sprintf((char *)rs232_tx_buf_gnss,"\ndynmodel: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 11);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nfixmode: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 10);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nfixedAlt: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 11);
//                print_char_line_from_spi(4);
                for (int i = 0; i < 4; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nfixedAltVar: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 14);
//                print_char_line_from_spi(4);
                for (int i = 0; i < 4; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nminElev: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 10);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ndrLimit: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 10);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\npDop: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 7);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ntDop: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 7);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\npAcc: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 7);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ntAcc: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 7);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nstaticHoldThresh: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 19);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ndgnssTimeout: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 15);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ncnoThreshNumSVs: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 18);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\ncnoThresh: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 12);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                // reserved1 field
                for (int i = 0; i < 2; ++i) cSLIM_spi_read_byte();

                //sprintf((char *)rs232_tx_buf_gnss,"\nstaticHoldMaxDist: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 20);
//                print_char_line_from_spi(2);
                for (int i = 0; i < 2; ++i) read_and_print_hex_from_spi();

                //sprintf((char *)rs232_tx_buf_gnss,"\nutcStandard: ");
                //rs232_transmit_string(rs232_tx_buf_gnss, 14);
//                print_char_line_from_spi(1);
                read_and_print_hex_from_spi();

                // reserved2 fields
                for (int i = 0; i < 5; ++i) cSLIM_spi_read_byte();

                printk( "\n\n");
                //rs232_transmit_string(rs232_tx_buf_gnss, 2);

                // Checksum. Not implemented any integrity validation, but necessary to read the whole data frame
                cSLIM_spi_dummy_read_n_byte(2);

                break_flag = true;
                break;
            }
            else ++retry;

            if(retry > RETRY){
                ret_flag = false;
                break;
            }
            k_sleep(K_MSEC(2));
        }
        if(break_flag == true) break;
    }
    if (outer_loop_var >= 50) ret_flag = false;
    cSLIM_spi_cs_release(GPS);
//    k_sleep(K_MSEC(2));
    k_sleep(K_MSEC(250));
    return ret_flag;
}


bool gps_enable_pps( void ) {
    //gps_int_pin_toggle();
    //sprintf((char *)rs232_tx_buf_gnss,"\nGPS_enable_pps called\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 25);
    //k_sleep(K_MSEC(2));
    bool flag_pps = send_cmd_rx_ack(cfg_tp5_pps_only_with_fix_new,  (sizeof(cfg_tp5_pps_only_with_fix_new)/sizeof(uint8_t)));
//    bool flag_save = send_cmd_rx_ack(ubx_cfg_cfg_save_nav_and_rxm, (sizeof(ubx_cfg_cfg_save_nav_and_rxm) / sizeof(uint8_t)));
//    set_gpio(AUX_3);
    return (flag_pps);
}


bool gps_disable_pps( void ) {
    gps_int_pin_toggle();
    //sprintf((char *)rs232_tx_buf_gnss,"\nGPS_disable_pps called\n");
    //rs232_transmit_string(rs232_tx_buf_gnss, 24);
    k_sleep(K_MSEC(2));
    bool flag_pps = send_cmd_rx_ack(cfg_tp5_disable,  (sizeof(cfg_tp5_disable)/sizeof(uint8_t)));
    bool flag_save = send_cmd_rx_ack(ubx_cfg_cfg_save_nav_and_rxm, (sizeof(ubx_cfg_cfg_save_nav_and_rxm) / sizeof(uint8_t)));
//    clear_gpio(AUX_3);
    return (flag_pps && flag_save);
}


bool spin_for_UBX_sync_byte( uint8_t retries ) {
    uint16_t retry = 0;
    //LOG_ERR("Spin");

    while ( cSLIM_spi_read_byte() != 0xB5 ) {
        if(retry++ > retries) 
        {
            //LOG_ERR("spin_for_UBX_sync_byte: Exceeded limit of retries");
            return false;
        }
        k_sleep(K_MSEC(1));
    }

    return true;
}


void gps_enter_backup_mode( void ) {
    send_cmd_no_validation(ubx_rxm_pmreq_inf, (sizeof(ubx_rxm_pmreq_inf)/sizeof(uint8_t)));
}



bool gps_change_pms_wakeup_interval( uint16_t wakeup_interval_in_sec ) {
	uint8_t msg[16];
    calculate_pms_wakeup_interval_msg(msg, wakeup_interval_in_sec);

    for (int i = 0; i < 10; ++i) {
        if (send_cmd_rx_ack(msg, (sizeof(msg) / sizeof(uint8_t)))) {
            if (send_cmd_rx_ack(ubx_cfg_cfg_save_nav_and_rxm, (sizeof(ubx_cfg_cfg_save_nav_and_rxm) / sizeof(uint8_t)))) {
                return true;
            }
        }
    }
    return false;
}


static void calculate_pms_wakeup_interval_msg( uint8_t* output_msg, uint16_t wakeup_interval_in_sec ) {
    output_msg[0] = SYNCH_1;
    output_msg[1] = SYNCH_2;
    output_msg[2] = CFG;
    output_msg[3] = PMS;
    output_msg[4] = 0x08;
    output_msg[5] = 0x00;
    output_msg[6] = 0x00;
    output_msg[7] = 0x02;

    output_msg[9] = ( wakeup_interval_in_sec >> 8 );
    output_msg[8] = ( wakeup_interval_in_sec & 0x00FF );

    output_msg[10] = 0x06;
    output_msg[11] = 0x00;
    output_msg[12] = 0x00;
    output_msg[13] = 0x00;

    uint16_t fletcher_crc = fletcher16(output_msg, 2, 12);
    output_msg[15] = ( fletcher_crc >> 8 );
    output_msg[14] = ( fletcher_crc & 0x00FF );
    k_sleep(K_MSEC(0));
}


void update_time_data_unix(nav_data_t* nav_data, unix_time_t* time_data) {
    time_data->unix_tstamp = time_manager_unixTimestamp( nav_data->year, nav_data->month, nav_data->day,
    													 nav_data->hour, nav_data->min, nav_data->sec );
    time_data->nano  = nav_data->nano;
    time_data->tAcc  = nav_data->tAcc;
    time_data->valid = nav_data->valid;

}

void gps_calculate_checksum(uint8_t* message, uint8_t message_length, uint8_t* ck_a, uint8_t* ck_b)
{
    *ck_a = 0; 
    *ck_b = 0;
    for(int i = 0; i < message_length; i++)
    {
        *ck_a   += message[i];
        *ck_b   += *ck_a;
    }
}