#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

__attribute__((__section__(".user_flash"))) static const uint8_t flash[128*1024];

HAL_StatusTypeDef flash_write(size_t address, uint8_t *data, size_t len) {
  HAL_StatusTypeDef status = HAL_OK;

  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase_config;
  erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase_config.Sector = 7;
  erase_config.NbSectors = 1;
  erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  uint32_t page_error;
  status = HAL_FLASHEx_Erase(&erase_config, &page_error);

  for(size_t i = 0; i < len && status == HAL_OK; i++) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t) &flash + i, data[i]);
  }

  HAL_FLASH_Lock();

  return status;
}

void flash_read(size_t address, uint8_t *data, size_t len) {
  memcpy(data, flash, len);
}
