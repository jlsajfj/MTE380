#include "compass.h"
#include "i2c.h"
#include "helper.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define COMPASS_ADDR 0x3C

typedef enum {
  COMPASS_STATE_RUN,
  COMPASS_STATE_CALIBRATE,
} compass_state_E;

static compass_state_E compass_state;
static uint8_t i2c_buff[6];
static bool buff_ready;

static int16_t max_x, max_y, min_x, min_y;
static double compass_heading;

void compass_init(void) {
  uint8_t config[] = {
    0x78, // 8 sample average, 75Hz data rate, no bias
    0x20, // 0.92 Mg/LSB
    0x00, // continuous measurement mode
  };

  if(HAL_I2C_Mem_Write(&hi2c1, COMPASS_ADDR, 0x0, I2C_MEMADD_SIZE_8BIT, config, sizeof(config), 100) != HAL_OK) {
    puts("compass init failed");
    return;
  }

  buff_ready = false;
  HAL_I2C_Mem_Read_IT(&hi2c1, COMPASS_ADDR, 0x3, I2C_MEMADD_SIZE_8BIT, i2c_buff, sizeof(i2c_buff));
}

void compass_run(void) {
  if(!buff_ready) return;

  int16_t x = (i2c_buff[0] << 8) | i2c_buff[1];
  int16_t y = (i2c_buff[4] << 8) | i2c_buff[5];

  buff_ready = false;
  HAL_I2C_Mem_Read_IT(&hi2c1, COMPASS_ADDR, 0x3, I2C_MEMADD_SIZE_8BIT, i2c_buff, sizeof(i2c_buff));

  switch(compass_state) {
    case COMPASS_STATE_RUN:
      min_x = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      min_y = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      max_x = config_get(CONFIG_ENTRY_COMPASS_MAX_X);
      max_y = config_get(CONFIG_ENTRY_COMPASS_MAX_X);
      break;

    case COMPASS_STATE_CALIBRATE:
      if(x < min_x) min_x = x;
      if(x > max_x) max_x = x;
      if(y < min_y) min_y = y;
      if(y > max_y) max_y = y;
      break;

    default:
      break;
  }

  double x_norm = NORMALIZE(x, min_x, max_x);
  double y_norm = NORMALIZE(y, min_y, max_y);

  compass_heading = atan2(y_norm - 0.5, x_norm - 0.5);
}

void compass_calibrate_start(void) {
  puts("starting calibration");
  min_x = min_y = 0x7FFF;
  max_x = max_y = -0x8000;
  compass_state = COMPASS_STATE_CALIBRATE;
}

void compass_calibrate_end(void) {
  printf("min %5d, %5d; max %5d, %5d\n", min_x, min_y, max_x, max_y);
  config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_x);
  config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_y);
  config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_x);
  config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_y);
  compass_state = COMPASS_STATE_RUN;
}

double compass_getHeading(void) {
  return compass_heading;
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  buff_ready = true;
}
