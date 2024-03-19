#ifndef __FLASH_H__
#define __FLASH_H__
#include "stm32f4xx_hal.h"

typedef enum {
	FLASH_ADDR_SPEED_POINTS = 0x0,
	FLASH_ADDR_CONFIG       = 0x100,
} flash_address_E;

HAL_StatusTypeDef flash_erase(void);
HAL_StatusTypeDef flash_write(flash_address_E address, void *data, size_t len);
void flash_read(flash_address_E flash_address_E, void *data, size_t len);

#endif
