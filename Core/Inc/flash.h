#ifndef __FLASH_H__
#define __FLASH_H__
#include "stm32f4xx_hal.h"

HAL_StatusTypeDef flash_write(size_t address, uint8_t *data, size_t len);
void flash_read(size_t address, uint8_t *data, size_t len);
#endif
