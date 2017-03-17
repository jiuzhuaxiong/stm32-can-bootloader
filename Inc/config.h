#ifndef _CONFIG_H
#define _CONFIG_H

#include "stm32f3xx_hal.h"


/////////////////////////////
// Flash
/////////////////////////////
//
// STM32F302 has:
//  - 2kbytes flash page size
//  - 64k flash total
//  => 32 pages total

#define NUM_PAGES_TOTAL 32U
#define MAIN_PROGRAM_PAGE_NUMBER   3U


/////////////////////////////
// CAN
/////////////////////////////
#define CAN_TRANSMIT_ADDRESS 0x02 // Respond on this address
#define CAN_LISTEN_ADDRESS 0x01 // Listen for packets on this address






typedef void (*pFunction)(void);

enum _prog_state {
	WAIT_HOST = 0,
	IDLE,
	PAGE_PROG,
};

enum _can_commands
{
	CMD_HOST_INIT = 0x01,
	CMD_PAGE_PROG = 0x02,
	CMD_BOOT = 0x03,
};

enum _can_responses
{
	CAN_RESP_OK = 0x01,
	CAN_RESP_ERROR = 0x02,
};


#define __hal_init HAL_Init
#endif
