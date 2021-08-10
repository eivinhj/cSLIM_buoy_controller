/*
 * ublox_msg.h
 *
 *  Created on: Apr 26, 2017
 *      Author: Waseemh
 *            : MariusSR
 */

#ifndef SRC_UBLOX_MSG_H_
#define SRC_UBLOX_MSG_H_

//command classes
#define 	NAV			0x01
#define 	RXM			0x02
#define 	INF			0x04
#define 	ACK			0x05
#define 	CFG			0x06
#define 	MON			0x0A
#define 	TIM			0x0D

//IDs for messages
#define 	MSG			0x01
#define 	DAT			0x06
#define 	PRT 		0x00
#define 	PM2 		0x3B
#define     PMS         0x86
#define 	RXM_CFG		0x11
#define 	SBAS		0x16
#define     TP5         0x31    // Time pulse parameters
#define 	PVT			0x07
#define     SAT         0x35
#define     GNSS        0x3e
#define     RATE        0x08
#define     TM2         0x03
#define     PMREQ       0x41
#define     NAV5        0x24

//Synch words
#define 	SYNCH_1		0xB5
#define 	SYNCH_2		0x62

// Fix types
#define     NO_FIX                 0
#define     DEAD_RECKONING ONLY    1
#define     TWO_DIM_FIX            2
#define     THREE_DIM_FIX          3
#define     GNSS_AND_RECKONING     4
#define     TIME_ONLY_FIX          5



/*
 * macros for sd card
 */

/*
 * typedefs
 */
/*
 * private constants
 */


static const uint8_t nav_pvt_gps_data[] = {     // p.326
		SYNCH_1, SYNCH_2, NAV, PVT, 0x00, 0x00,
		0x08, 0x19		//checksum (a and b)
		};


static const uint8_t nav_sat_gps_data[] = {     // p.331
        SYNCH_1, SYNCH_2, NAV, SAT, 0x00, 0x00,
        0x36, 0xA3		//checksum (a and b)
        };


static const uint8_t cfg_prt_spi[] = {
		SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
		0x04,							//port id
		0x00,							//reserved
		0x00,	0x00,					//txready
		0x00,	0x32,	0x00,	0x00,	//spi mode
		0x00,	0x00,	0x00,	0x00,	//reserved
		0x01,	0x00,					//inportmask=disable NMEA
		0x01,	0x00,					//outportmask=disable NMEA
		0x00,	0x00,					//flags
		0x00,	0x00,					//reserved
		0x52,	0x94					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
//        0x35,	0x00,					//txready on EXTINT (PIO13)
        0xB5,	0x00,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
//        0x87,	0x4e					//checksum (a and b)
        0x07,	0x4e					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_inverted[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
//        0x35,	0x00,					//txready on EXTINT (PIO13)
//        0xB5,	0x00,					//txready on EXTINT (PIO13)
        0xB7,	0x00,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
//        0x87,	0x4e					//checksum (a and b)
        0x09,	0x72					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_8_byte_threshold[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB5,	0x01,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x08,	0x5f					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_inverted_8_byte_threshold[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB7,	0x01,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x0a,	0x83					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_inverted_extended_timeout[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB7,	0x00,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x02,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x0b,	0x7a					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_extended_timeout[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB5,	0x00,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x02,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x09,	0x56					//checksum (a and b)
};


static const uint8_t cfg_prt_spi_msg_ready_interrupt_inverted_extended_timeout_8_byte_threshold[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB7,	0x01,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x02,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x0c,	0x8b					//checksum (a and b)
};


static const uint8_t cfg_prt_cSLIM_spi_msg_ready_interrupt_extended_timeout_8_byte_threshold[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x04,							//port id
        0x00,							//reserved
        0xB5,	0x01,					//txready on EXTINT (PIO13)
        0x00,	0x32,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x01,	0x00,					//inportmask=disable NMEA
        0x01,	0x00,					//outportmask=disable NMEA
        0x02,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x0a,	0x67					//checksum (a and b)
};


static const uint8_t cfg_prt_poll[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x00, 0x00,
        0x06,	0x18					//checksum (a and b)
};


static const uint8_t cfg_prt_poll_uart1[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x01,                            // port, uart 1
        0x08,	0x22					 // checksum (a and b)
};


static const uint8_t cfg_prt_poll_uart2[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x02,                            // port, uart 2
        0x09,	0x23					 // checksum (a and b)
};


static const uint8_t cfg_prt_poll_uart5[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x05,                            // port, uart 5
        0x0c,	0x26					 // checksum (a and b)
};


static const uint8_t cfg_prt_poll_ddc[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x00,                            // port, ddc
        0x07,	0x21					 // checksum (a and b)
};


static const uint8_t cfg_prt_poll_usb[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x03,                            // port, usb
        0x0a,	0x24					 // checksum (a and b)
};


static const uint8_t cfg_prt_poll_spi[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x01, 0x00,
        0x04,                            // port, spi
        0x0b,	0x25					 // checksum (a and b)
};


static const uint8_t cfg_msg_nav_pvt_over_cSLIM_spi_enable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        NAV,                            // msgClass
        PVT,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x01,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x18,	0xde					// checksum (a and b)
};


static const uint8_t cfg_msg_nav_pvt_over_cSLIM_spi_disable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        NAV,                            // msgClass
        PVT,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x00,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x17,	0xdc					// checksum (a and b)
};


static const uint8_t cfg_msg_tim_tp_over_cSLIM_spi_enable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        TIM,                            // msgClass
        MSG,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x01,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x1e,	0x14					// checksum (a and b)
};



static const uint8_t cfg_msg_tim_tp_over_cSLIM_spi_disable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        TIM,                            // msgClass
        MSG,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x00,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x1d,	0x12					// checksum (a and b)
};


static const uint8_t cfg_msg_tim_tm2_over_cSLIM_spi_enable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        TIM,                            // msgClass
        TM2,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x01,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x20,	0x22					// checksum (a and b)
};


static const uint8_t cfg_msg_tim_tm2_over_cSLIM_spi_disable[] = {
        SYNCH_1, SYNCH_2, CFG, MSG, 0x08, 0x00,
        TIM,                            // msgClass
        TM2,                            // msgId
        0x00,                           // relative rate on DDC port
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on UART ?
        0x00,                           // relative rate on USB port
        0x00,                           // relative rate on SPI port
        0x00,                           // relative rate on UART ?
        0x1f,	0x20					// checksum (a and b)
};


static const uint8_t cfg_prt_uart[] = {
		SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
		0x01,							//port id
		0x00,							//reserved
		0x00,	0x00,					//txready
		0x00,	0x00,	0x00,	0x00,	//spi mode
		0x00,	0x00,	0x00,	0x00,	//reserved
		0x00,	0x00,					//inportmask=disable all
		0x00,	0x00,					//outportmask=disable all
		0x00,	0x00,					//flags
		0x00,	0x00,					//reserved
		0x1b,	0x5c					//checksum (a and b)
};

static const uint8_t cfg_prt_uart2[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x02,							//port id
        0x00,							//reserved
        0x00,	0x00,					//txready
        0x00,	0x00,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x00,	0x00,					//inportmask=disable all
        0x00,	0x00,					//outportmask=disable all
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x1c,	0x70					//checksum (a and b)
};

static const uint8_t cfg_prt_uart5[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x05,							//port id
        0x00,							//reserved
        0x00,	0x00,					//txready
        0x00,	0x00,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x00,	0x00,					//inportmask=disable all
        0x00,	0x00,					//outportmask=disable all
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x1f,	0xac					//checksum (a and b)
};

static const uint8_t cfg_prt_uart6[] = {
        SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
        0x06,							//port id
        0x00,							//reserved
        0x00,	0x00,					//txready
        0x00,	0x00,	0x00,	0x00,	//spi mode
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x00,	0x00,					//inportmask=disable all
        0x00,	0x00,					//outportmask=disable all
        0x00,	0x00,					//flags
        0x00,	0x00,					//reserved
        0x20,	0xc0					//checksum (a and b)
};


static const uint8_t cfg_prt_usb[] = {
		SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
		0x03,							//port id
		0x00,							//reserved
		0x00,	0x00,					//txready
		0x00,	0x00,	0x00,	0x00,	//spi mode
		0x00,	0x00,	0x00,	0x00,	//reserved
		0x00,	0x00,					//inportmask=disable all
		0x00,	0x00,					//outportmask=disable all
		0x00,	0x00,					//flags
		0x00,	0x00,					//reserved
		0x1d,	0x84					//checksum (a and b)
};


static const uint8_t cfg_prt_ddc[] = {
		SYNCH_1, SYNCH_2, CFG, PRT, 0x14, 0x00,
		0x00,							//port id
		0x00,							//reserved
		0x00,	0x00,					//txready
		0x00,	0x00,	0x00,	0x00,	//spi mode
		0x00,	0x00,	0x00,	0x00,	//reserved
		0x00,	0x00,					//inportmask=disable all
		0x00,	0x00,					//outportmask=disable all
		0x00,	0x00,					//flags
		0x00,	0x00,					//reserved
		0x1a,	0x48					//checksum (a and b)
};


// Power saving configurations
static const uint8_t cfg_pm2_on_off_external_ctrl[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x30, 0x00,
        0x02,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0x60,	0x04,	0x00,	0x00,	// flags =  ExtIntBackup, ExtIntWake, waitTimeFix
        0xD8,	0xD6,	0x00,	0x00,	// updatePeriod = 55sec
        0x60,	0xEA,	0x00,	0x00,	// seacrhPeriod = 60sec
        0x00,	0x00,	0x00,	0x00,	// gridOffset = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x05,	0x00,					// minAcqTime = 5 sec
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x30,	0x75,	0x00,	0x00,	// extintInactivityMs = 30s
        0x7f,	0x06					// checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_force_on_int[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x30, 0x00,
        0x02,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0xA0,	0x04,	0x00,	0x00,	// flags =  ExtIntBackup, ExtIntWake, ExtIntInactive, waitTimeFix
        0xD8,	0xD6,	0x00,	0x00,	// updatePeriod = 55sec
        0x60,	0xEA,	0x00,	0x00,	// seacrhPeriod = 60sec
        0x00,	0x00,	0x00,	0x00,	// gridOffset = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x05,	0x00,					// minAcqTime = 5 sec
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x30,	0x75,	0x00,	0x00,	// extintInactivityMs = 30s
        0xbf,	0x06					// checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_update_on_ext_int[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x2C, 0x00,
        0x01,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0x00,	0x14,	0x00,	0x00,	// flags =  UpdateEPH, waitTimeFix
        0x00,	0x00,	0x00,	0x00,	// updatePeriod = 0sec
        0x00,	0x00,	0x00,	0x00,	// seacrhPeriod = 0sec
        0x00,	0x00,	0x00,	0x00,	// gridOffset   = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x05,	0x00,					// minAcqTime = 5 sec
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x8d,	0x13					// checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_no_int_ctrl_donotenteroff[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x30, 0x00,
        0x02,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0x00,	0x14,	0x01,	0x00,	// flags
        0x00,	0x00,	0x00,	0x00,	// updatePeriod = 0 sec. waits for external events
        0x10,	0x27,	0x00,	0x00,	// seacrhPeriod = 10 sec. Not in use
        0x00,	0x00,	0x00,	0x00,	// gridOffset = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x00,	0x00,					// minAcqTime = 0 sec. Automatically chosen by receiver
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// extintInactivityMs = 0 sec. Not in use
        0xc5,	0x7c					// checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_no_int_ctrl[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x30, 0x00,
        0x02,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0x00,	0x04,	0x00,	0x00,	// flags
        0xD8,	0xD6,	0x00,	0x00,	// updatePeriod = 55sec
        0x60,	0xEA,	0x00,	0x00,	// seacrhPeriod = 60sec
        0x00,	0x00,	0x00,	0x00,	// gridOffset = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x05,	0x00,					// minAcqTime = 5 sec
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// extintInactivityMs
        0x7a,	0x67					// checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_extintBackup[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x2C, 0x00,
        0x01,							//msgVersion
        0x00,							//reserved
        0x00,							//reserved
        0x00,							//reserved
        0x40,	0x0C,	0x00,	0x00,	//flags =Update RTC & wait for timeFix, ON/OFF mode & extIntBackup
        0xD8,	0xD6,	0x00,	0x00,	//updatePeriod=55sec
        0x60,	0xEA,	0x00,	0x00,	//seacrhPeriod=60sec
        0x00,	0x00,	0x00,	0x00,	//gridOffset=0 msec
        0x06,	0x00,					//onTime=6 sec
        0x05,	0x00,					//minAcqTime=5 sec
        0x00,	0x00,					//reserved
        0x00,	0x00,					//reserved
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x00,	0x00,	0x00,	0x00,	//reserved
        0x00,							//reserved
        0x00,							//reserved
        0x00,	0x00,					//reserved
        0x00,	0x00,	0x00,	0x00,	//reserved
        0xBD,	0xD3					//checksum (a and b)
};


static const uint8_t cfg_pm2_on_off_force_off_int[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x30, 0x00,
        0x02,							// msgVersion
        0x00,							// reserved
        0x00,							// masStartupDuration
        0x00,							// reserved2
        0x40,	0x14,	0x00,	0x00,	// flags =  ExtIntBackup, ExtIntWake, ExtIntInactive, waitTimeFix
        0xE0,	0x93,	0x04,	0x00,	// updatePeriod = 300sec
        0x30,	0x75,	0x00,	0x00,	// seacrhPeriod = 30sec
        0x00,	0x00,	0x00,	0x00,	// gridOffset = 0 msec
        0x06,	0x00,					// onTime = 6 sec
        0x00,	0x00,					// minAcqTime = 5 sec
        0x00,	0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x00,   0x00,   0x00,	0x00,   // reserved
        0x00,	0x00,	0x00,	0x00,	// reserved
        0x30,	0x75,	0x00,	0x00,	// extintInactivityMs = 30s
        0x8e,	0x98					// checksum (a and b)
};


static const uint8_t cfg_pm2_poll_on_off_extintBackup[] = {
        SYNCH_1, SYNCH_2, CFG, PM2, 0x00, 0x00,
        0x41,	0xC9					//checksum (a and b)
};


static const uint8_t cfg_sbas_disable[] = {
		SYNCH_1, SYNCH_2, CFG, SBAS, 0x08, 0x00,
		0x00,							//mode=disable
		0x00,							//usage
		0x00,							//maxSBAS
		0x00,							//scanMode2
		0x00,	0x00,	0x00,	0x00,	//scanMode1
		0x24,	0x8A					//checksum (a and b)
};


static const uint8_t cfg_rxm_psm_mode[] = {
		SYNCH_1, SYNCH_2, CFG, RXM_CFG, 0x02, 0x00,
		0x00,							//reserved
		0x01,							//lpmode = power save
		0x1A,	0x82					//checksum (a and b)
};


static const uint8_t cfg_rxm_psm_continuous_mode[] = {
        SYNCH_1, SYNCH_2, CFG, RXM_CFG, 0x02, 0x00,
        0x00,							//reserved
        0x00,							//lpmode = cont mode
        0x19,	0x81					//checksum (a and b)
};


static const uint8_t cfg_rxm_poll_psm[] = {
		SYNCH_1, SYNCH_2, CFG, RXM_CFG, 0x00, 0x00,
		0x17,	0x4b					//checksum (a and b)
};


static const uint8_t cfg_pms_interval_60s[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x3c, 0x00,                     // period: 60s
        0x06, 0x00,                     // onTime: 6s
        0x00, 0x00,                     // reserved
        0xd8, 0xe8,                     // checksum (a and b)
};


static const uint8_t cfg_pms_interval_120s[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x78, 0x00,                     // period: 120s
        0x06, 0x00,                     // onTime: 6s
        0x00, 0x00,                     // reserved
        0x14, 0x50,                     // checksum (a and b)
};


static const uint8_t cfg_pms_interval_300s[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x2c, 0x01,                     // period: 300s
        0x06, 0x00,                     // onTime: 6s
        0x00, 0x00,                     // reserved
        0xc9, 0x8d,                     // checksum (a and b)
};


static const uint8_t cfg_pms_interval_3600s_6s_on_time[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x10, 0x0e,                     // period: 3600s
        0x06, 0x00,                     // onTime: 6s
        0x00, 0x00,                     // reserved
        0xba, 0x26,                     // checksum (a and b)
};


static const uint8_t cfg_pms_interval_3600s_3s_on_time[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x10, 0x0e,                     // period: 3600s
        0x03, 0x00,                     // onTime: 3s
        0x00, 0x00,                     // reserved
        0xb7, 0x1a,                     // checksum (a and b)
};


static const uint8_t cfg_pms_interval_3600s_5s_on_time[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x10, 0x0e,                     // period: 3600s
        0x05, 0x00,                     // onTime: 5s
        0x00, 0x00,                     // reserved
        0xb9, 0x22,                     // checksum (a and b)
};

static  const uint8_t cfg_pms_interval_60s_1s_onTime[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x02,                           // powerSetupValue: interval
        0x3c, 0x00,                     // period: 60s
        0x01, 0x00,                     // onTime: 6s
        0x00, 0x00,                     // reserved
        0xd3, 0xd4,                     // checksum (a and b)
};


static  const uint8_t cfg_pms_balanced[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
        0x00,                           // version
        0x01,                           // powerSetupValue: balanced
        0x00, 0x00,                     // period: not in use
        0x00, 0x00,                     // onTime: not in use
        0x00, 0x00,                     // reserved
        0x95, 0x61,                     // checksum (a and b)
};


static  const uint8_t cfg_pms_full_power[] = {
		SYNCH_1, SYNCH_2, CFG, PMS, 0x08, 0x00,
		0x00,                           // version
		0x00,                           // powerSetupValue: full power
		0x00, 0x00,                     // period: not in use
		0x00, 0x00,                     // onTime: not in use
		0x00, 0x00,                     // reserved
		0x94, 0x5a,                     // checksum (a and b)
};


static  const uint8_t cfg_pms_poll[] = {
        SYNCH_1, SYNCH_2, CFG, PMS, 0x00, 0x00,
        0x8C, 0xAA,                     // checksum (a and b)
};


/////////////////////
static const uint8_t cfg_gnss[] = {
		SYNCH_1, SYNCH_2, CFG, GNSS, 0x3C, 0x00,
		0x00,	0x20,	0x20,	0x07,								    // first4 bytes
    // gnssID resTrkCh maxTrkCh reserv | enable | SigCfgMask
//		0x00,	0x08,	0x10,	0x00,	0x01,     0x00,   0x01,    0x00, 	//  GPS settings
		0x00,	0x08,	0x20,	0x00,	0x01,     0x00,   0x01,    0x00, 	//  GPS settings
		0x01,	0x01,	0x03,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  SBAS
		0x02,	0x04,	0x08,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  Galileo
		0x03,	0x08,	0x10,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  Beidou
		0x04,	0x00,	0x08,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  IMES
		0x05,	0x00,	0x03,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  QZSS
		0x06,	0x08,	0x0e,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  GLONASS
//		0x3f,	0x7a									                // checksum (a and b)
		0x4f,	0xda									                // checksum (a and b)
};


/////////////////////
static const uint8_t cfg_gnss_gps_glonass[] = {
		SYNCH_1, SYNCH_2, CFG, GNSS, 0x3C, 0x00,
		0x00,	0x20,	0xff,	0x07,						// first4 bytes
		0x00,	0x08,	0x20,	0x00,	0x01,     0x00,   0x01,    0x00, 	//  GPS settings
		0x01,	0x01,	0x03,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  SBAS
		0x02,	0x04,	0x08,	0x00,	0x01,     0x00,   0x01,    0x00, 	//  Galileo
		0x03,	0x08,	0x10,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  Beidou
		0x04,	0x00,	0x08,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  IMES
		0x05,	0x00,	0x03,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  QZSS
		0x06,	0x08,	0x0e,	0x00,	0x00,     0x00,   0x00,    0x00, 	//  GLONASS
//		0x3f,	0x7a									                // checksum (a and b)
		0x30,	0xa6										                // checksum (a and b)
};


static const uint8_t cfg_gnss_poll[] = {
		SYNCH_1, SYNCH_2, CFG, 0x3E, 0x00, 0x00,
		0x44,	0xd2								//first4 bytes
};

static const uint8_t cfg_rst_gnss_enable_hot_start[] = {
		SYNCH_1, SYNCH_2, CFG, 0x04, 0x04, 0x00,
		0x00,           							//Config HOT start (should be valid for 4 hours, if not use warm start (0x01) or cold start(0xff))
                0x08,                                                                   //Enable gnss
                0x00,                                                                    //reserved
                0x16, 0x66                                                              //Checksum

};

static const uint8_t cfg_rst_gnss_enable_warm_start[] = {
		SYNCH_1, SYNCH_2, CFG, 0x04, 0x04, 0x00,
		0x01,           							//Config HOT start (should be valid for 4 hours, if not use warm start (0x01) or cold start(0xff))
                0x08,                                                                   //Enable gnss
                0x00,                                                                    //reserved
                0x17, 0x69                                                              //Checksum

};

static const uint8_t cfg_rst_gnss_enable_cold_start[] = {
		SYNCH_1, SYNCH_2, CFG, 0x04, 0x04, 0x00,
		0xFF,           							//Config HOT start (should be valid for 4 hours, if not use warm start (0x01) or cold start(0xff))
                0x08,                                                                   //Enable gnss
                0x00,                                                                   //reserved
                0x15, 0x63                                                              //Checksum

};

static const uint8_t cfg_rst_gnss_disable[] = {
		SYNCH_1, SYNCH_2, CFG, 0x04, 0x04, 0x00,
		0x00,           							//First byte
                0x09,                                                                   //Enable gnss
                0x00,                                                                    //reserved
                0x17, 0x68                                                              //Checksum
                
};


static const uint8_t cfg_tp5_poll[] = {
		SYNCH_1, SYNCH_2, CFG, 0x31, 0x00, 0x00,
		0x37,	0xab
};


static const uint8_t cfg_tp5_disable[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
        0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
        0x58,	0x3e,	0x0f,	0x00,	                                // PulseLenratio = 999 000
        0x58,	0x3e,	0x0f,	0x00,                                   // PulseLenRatioLock = 999 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0xb6,	0x00,	0x00,	0x00,                                   // flags
        0xac,	0xba
};


static const uint8_t cfg_tp5[] = {
		SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
                0x00,                                                           // tpIdx = TIMEPULSE
		0x01,                                                           // version
		0x00,	0x00,                                                   // reserved
		0x32,	0x00,                                                   // antenna cable delay
		0x00,	0x00,                                                   // RF group delay
		0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
		0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
		0xa0,	0x86,	0x01,	0x00,	                                // PulseLenratio = PulseLenRatioLock = 100 000
		0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000
		0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
		0xf7,	0x00,	0x00,	0x00,                                   // flags
		0xf1,	0x9e                                                    //Checksum
};


static const uint8_t cfg_tp5_pps_without_fix[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
        0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
        0xa0,	0x86,	0x01,	0x00,	                                // PulseLenratio = PulseLenRatioLock = 100 000
        0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0xd1,	0x00,	0x00,	0x00,                                   // flags
        0xcb,	0x06                                                    //Checksum
};


static const uint8_t cfg_tp5_pps_only_with_fix[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
        0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
        0x00,	0x35,	0x0C,	0x00,	                                // PulseLenratio = 800 000
        0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0xf7,	0x00,	0x00,	0x00,                                   // flags: GPS time
        0x0b,	0x79                                                    // Checksum
};



static const uint8_t cfg_tp5_pps_only_with_fix_new[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x01,	0x00,	0x00,	0x00,	                                // freqPeriod  = 1 hz
        0x01,	0x00,	0x00,	0x00,                                   // freqPeriodLock = 1 Hz
        0x00,	0x00,	0x00,	0x00,	                                // PulseLenRatio = 0
        0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000 us
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns
        0xff,	0x20,	0x00,	0x00,                                   // flags: GPS time
        0xd2,	0x36                                                    // Checksum
};

static const uint8_t cfg_tp5_pps_only_with_fix_60s[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x00,	0x87,	0x93,	0x03,	                                // periodTime  = 60s,
        0x00,	0x87,	0x93,	0x03,	                                // periodTimeLock  = 60s,
        0x00,	0x35,	0x0C,	0x00,	                                // PulseLenratio = 800 000
        0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0xf7,	0x00,	0x00,	0x00,                                   // flags: GPS time
        0x23,	0xdd                                                    // Checksum
};


static const uint8_t cfg_tp5_pps_only_with_fix_utc_time_base[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
        0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
        0x00,	0x35,	0x0C,	0x00,	                                // PulseLenratio = 800 000
        0xa0,	0x86,	0x01,	0x00,                                   // PulseLenRatioLock = 100 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0x77,	0x00,	0x00,	0x00,                                   // flags: UTC time
        0x8b,	0x79
};


static const uint8_t cfg_tp5_inversed_polarity[] = {
        SYNCH_1, SYNCH_2, CFG, 0x31, 0x20, 0x00,
        0x00,                                                           // tpIdx = TIMEPULSE
        0x01,                                                           // version
        0x00,	0x00,                                                   // reserved
        0x32,	0x00,                                                   // antenna cable delay
        0x00,	0x00,                                                   // RF group delay
        0x40,	0x42,	0x0f,	0x00,	                                // freqPeriod  = 1 000 000,
        0x40,	0x42,	0x0f,	0x00,                                   // freqPeriodLock = 1 000 000
        0x58,	0x3e,	0x0f,	0x00,	                                // PulseLenratio = 999 000
        0x58,	0x3e,	0x0f,	0x00,                                   // PulseLenRatioLock = 999 000
        0x00,	0x00,	0x00,	0x00,                                   // userConfigDelay = 0 ns,
        0xf7,	0x00,	0x00,	0x00,                                   // flags
        0xed,	0xbe
};


static const uint8_t ubx_mon_ver[] = {
        SYNCH_1, SYNCH_2, 0x0A, 0x04, 0x00, 0x00,
        0x0E, 0x34
};


static const uint8_t ubx_cfg_cfg_clear_and_load_nav_and_rxm[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x18,	0x00,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x18,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x63, 0x80
};


static const uint8_t ubx_cfg_cfg_save_nav_and_rxm[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x00,	0x00,	0x00,	0x00,       // clear mask
        0x18,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x4b, 0xa8
};


static const uint8_t ubx_cfg_cfg_load_nav_and_rxm[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x00,	0x00,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x18,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x4b, 0x48
};


static const uint8_t ubx_cfg_cfg_save_ioPort[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x00,	0x00,	0x00,	0x00,       // clear mask
        0x01,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x34, 0xd9
};


static const uint8_t ubx_cfg_cfg_clear_ioPort[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x01,	0x00,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x34, 0xdd
};


static const uint8_t ubx_cfg_cfg_save_msg[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x00,	0x00,	0x00,	0x00,       // clear mask
        0x02,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x35, 0xe2
};


static const uint8_t ubx_cfg_cfg_clear_msg[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x02,	0x00,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x35, 0xea
};


static const uint8_t ubx_cfg_cfg_save_ioPort_and_msg[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x00,	0x00,	0x00,	0x00,       // clear mask
        0x03,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x36, 0xeb
};


static const uint8_t ubx_cfg_cfg_clear_ioPort_and_msg[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x03,	0x00,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x00,	0x00,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0x36, 0xf7
};


static const uint8_t ubx_cfg_cfg_clear_and_load_all[] = {
        SYNCH_1, SYNCH_2, CFG, 0x09, 0x0d, 0x00,
        0x1f,	0x1f,	0x00,	0x00,       // clear mask
        0x00,	0x00,	0x00,	0x00,       // save  mask
        0x1f,	0x1f,	0x00,	0x00,       // load  mask
        0x17,                               // device mask
        0xaf, 0xee
};


///////////////////////////  TIME PULSE
static const uint8_t ubx_cfg_rate_1s[] = {
        SYNCH_1, SYNCH_2, CFG, RATE, 0x06, 0x00,
        0xe8, 0x03,                         // measRate: 1Hz
        0x01, 0x00,                         // navRate: 1 measurment
        0x00, 0x00,                         // timeRef: UTC time
        0x00, 0x37
};

static const uint8_t ubx_cfg_rate_1s_gps_time[] = {
        SYNCH_1, SYNCH_2, CFG, RATE, 0x06, 0x00,
        0xe8, 0x03,                         // measRate: 1Hz
        0x01, 0x00,                         // navRate: 1 measurment
        0x01, 0x00,                         // timeRef: UTC time
        0x01, 0x39
};


static const uint8_t ubx_cfg_rate_30s[] = {
        SYNCH_1, SYNCH_2, CFG, RATE, 0x06, 0x00,
        0x30, 0x75,                         // measRate: 1/30 Hz
        0x01, 0x00,                         // navRate: 1 measurment
        0x00, 0x00,                         // timeRef: UTC time
        0xba, 0x21
};


static const uint8_t ubx_cfg_rate_60s[] = {
        SYNCH_1, SYNCH_2, CFG, RATE, 0x06, 0x00,
        0x60, 0xea,                         // measRate: 1/60 Hz
        0x01, 0x00,                         // navRate: 1 measurment
        0x00, 0x00,                         // timeRef: UTC time
        0x5f, 0x8a
};


static const uint8_t ubx_tim_tp_poll[] = {
        SYNCH_1, SYNCH_2, TIM, MSG, 0x00, 0x00,
        0x0e, 0x37
};


// RXM
static const uint8_t ubx_rxm_pmreq_inf[] = {
        SYNCH_1, SYNCH_2, RXM, PMREQ, 0x10, 0x00,
        0x00,                               // version
        0x00, 0x00, 0x00,                   // reserved
        0x00, 0x00, 0x00, 0x00,             // duration
        0x06, 0x00, 0x00, 0x00,             // flags
        0xe0, 0x00, 0x00, 0x00,             // wakeup sources
        0x39, 0xcb
};

static const uint8_t ubx_rxm_pmreq_10s[] = {
        SYNCH_1, SYNCH_2, RXM, PMREQ, 0x10, 0x00,
        0x00,                               // version
        0x00, 0x00, 0x00,                   // reserved
        0x10, 0x27, 0x00, 0x00,             // duration
        0x06, 0x00, 0x00, 0x00,             // flags
        0xe0, 0x00, 0x00, 0x00,             // wakeup sources
        0x70, 0x38
};

static const uint8_t ubx_rxm_pmreq_30s[] = {
        SYNCH_1, SYNCH_2, RXM, PMREQ, 0x10, 0x00,
        0x00,                               // version
        0x00, 0x00, 0x00,                   // reserved
        0x30, 0x75, 0x00, 0x00,             // duration
        0x06, 0x00, 0x00, 0x00,             // flags
        0xe0, 0x00, 0x00, 0x00,             // wakeup sources
        0xde, 0x12
};


static const uint8_t ubx_rxm_pmreq_60s[] = {
        SYNCH_1, SYNCH_2, RXM, PMREQ, 0x10, 0x00,
        0x00,                               // version
        0x00, 0x00, 0x00,                   // reserved
        0x60, 0xea, 0x00, 0x00,             // duration
        0x06, 0x00, 0x00, 0x00,             // flags
        0xe0, 0x00, 0x00, 0x00,             // wakeup sources
        0x83, 0x59
};


// NAV5
static const uint8_t ubx_nav5_get[] = {
        SYNCH_1, SYNCH_2, CFG, NAV5, 0x00, 0x00,
        0x2a, 0x84
};


static const uint8_t ubx_nav5_stationary_model[] = {
        SYNCH_1, SYNCH_2, CFG, NAV5, 0x24, 0x00,
        0x01, 0x04,                         // mask
        0x02,                               // dynModel = stationary
        0x00,                               // fixMode,  do not apply with mask. Factory calibrated!
        0x00, 0x00, 0x00, 0x00,             // fixedAlt, do not apply with mask. Factory calibrated!
        0x00, 0x00, 0x00, 0x00,             // fixedAltVar, do not apply with mask. Factory calibrated!
        0x00,                               // minElev,  do not apply with mask. Factory calibrated!
        0x00,                               // reserved
        0x00, 0x00,                         // pDop
        0x00, 0x00,                         // tDop
        0x00, 0x00,                         // pAcc [m]
        0x00, 0x00,                         // tAcc [m]
        0x00,                               // staticHoldThreshold
        0x00,                               // dgnssTimeout
        0x00,                               // cnoThresholdNumSVs, do not apply with mask. Factory calibrated!
        0x00,                               // cnoThresh, do not apply with mask. Factory calibrated!
        0x00, 0x00,                         // reserved
        0x00, 0x00,                         // staticHoldMaxDist
        0x03,                               // utcStandard
        0x00, 0x00, 0x00, 0x00, 0x00,       // reserved
        0x58, 0xca,                         // checksum (a and b)
};


static const uint8_t ubx_nav5_stationary_model_with_timemask[] = {
        SYNCH_1, SYNCH_2, CFG, NAV5, 0x24, 0x00,
//        0x21, 0x00,                         // mask
        0x21, 0x04,                         // mask
        0x02,                               // dynModel = stationary
        0x00,                               // fixMode,  do not apply with mask. Factory calibrated!
        0x00, 0x00, 0x00, 0x00,             // fixedAlt, do not apply with mask. Factory calibrated!
        0x00, 0x00, 0x00, 0x00,             // fixedAltVar, do not apply with mask. Factory calibrated!
        0x00,                               // minElev,  do not apply with mask. Factory calibrated!
        0x00,                               // reserved
        0x00, 0x00,                         // pDop
        0xc4, 0x09,                         // tDop
        0x00, 0x00,                         // pAcc [m]
        0xAC, 0x0D,                         // tAcc [m]
        0x00,                               // staticHoldThreshold
        0x00,                               // dgnssTimeout
        0x00,                               // cnoThresholdNumSVs, do not apply with mask. Factory calibrated!
        0x00,                               // cnoThresh, do not apply with mask. Factory calibrated!
        0x00, 0x00,                         // reserved
        0x00, 0x00,                         // staticHoldMaxDist
        0x03,                               // utcStandard
        0x00, 0x00, 0x00, 0x00, 0x00,       // reserved
        0xfe, 0xc8,                         // checksum (a and b)
};

static const uint8_t ubx_cfg_txslot_msg_tp_enable_uart[] = {
        SYNCH_1, SYNCH_2, CFG, 0x53, 0x10, 0x00,
        0x00,                                   //Version=0
        0x10,                                   //Enable for SPI
        0x00,                                   //Use Timepulse as referencessource
        0x00,                                   //reserved
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0x75, 0x6d,                             //Checksum
};

static const uint8_t ubx_cfg_txslot_msg_tp_disable[] = {
        SYNCH_1, SYNCH_2, CFG, 0x53, 0x10, 0x00,
        0x00,                                   //Version=0
        0x00,                                   //Disable
        0x00,                                   //Use Timepulse as referencessource
        0x00,                                   //reserved
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0xFF, 0x00, 0x00, 0x00 ,                //end of timeslot, repeated
        0x36, 0x04,                             //Checksum
};



#endif /* SRC_UBLOX_MSG_H_ */
