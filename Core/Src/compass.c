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

static compass_state_E compass_state = COMPASS_STATE_IDLE;
static compass_state_E compass_state_next = COMPASS_STATE_IDLE;

static uint8_t i2c_buff[6];
static bool buff_ready;

static double x = 0.0, y = 0.0;
static double max_x, max_y, min_x, min_y;
static double compass_heading;
static double alpha;

static void compass_read(void);

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
  if(compass_state_next != compass_state) {
    switch(compass_state) {
      case COMPASS_STATE_CALIBRATE:
        printf("min %5lf, %5lf; max %5lf, %5lf\n", min_x, min_y, max_x, max_y);
        config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_x);
        config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_y);
        config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_x);
        config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_y);
        alpha = 0.0;
        break;

      default:
        break;
    }

    compass_state = compass_state_next;

    switch(compass_state) {
      case COMPASS_STATE_CALIBRATE:
        puts("starting calibration");
        min_x = min_y = INFINITY;
        max_x = max_y = -INFINITY;
        alpha = 0.8;
        break;

      default:
        break;
    }
  }

  switch(compass_state) {
    case COMPASS_STATE_RUN:
      compass_read();
      min_x = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      min_y = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      max_x = config_get(CONFIG_ENTRY_COMPASS_MAX_X);
      max_y = config_get(CONFIG_ENTRY_COMPASS_MAX_X);

      double x_norm = NORMALIZE(x, min_x, max_x);
      double y_norm = NORMALIZE(y, min_y, max_y);

      compass_heading = atan2(y_norm - 0.5, x_norm - 0.5);

      break;

    case COMPASS_STATE_CALIBRATE:
      compass_read();
      if(x < min_x) min_x = x;
      if(x > max_x) max_x = x;
      if(y < min_y) min_y = y;
      if(y > max_y) max_y = y;
      break;

    default:
      break;
  }
}

void compass_setState(compass_state_E state) {
  compass_state_next = state;
}

double compass_getHeading(void) {
  return compass_heading;
}

static void compass_read(void) {
  if(!buff_ready) return;

  x = alpha * x + (1 - alpha) * (int16_t) ((i2c_buff[0] << 8) | i2c_buff[1]);
  y = alpha * y + (1 - alpha) * (int16_t) ((i2c_buff[4] << 8) | i2c_buff[5]);

  buff_ready = false;
  HAL_I2C_Mem_Read_IT(&hi2c1, COMPASS_ADDR, 0x3, I2C_MEMADD_SIZE_8BIT, i2c_buff, sizeof(i2c_buff));
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  buff_ready = true;
}
