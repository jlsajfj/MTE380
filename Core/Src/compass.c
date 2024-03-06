#include "compass.h"
#include "i2c.h"
#include "helper.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define COMPASS_ADDR 0x6A

typedef enum {
  COMPASS_STATE_RUN,
  COMPASS_STATE_CALIBRATE,
} compass_state_E;

static compass_state_E compass_state;
static uint8_t i2c_buff[6];
static volatile bool buff_ready;

static double max_x, max_y, min_x, min_y;
static double x = 0.0, y = 0.0, z = 0.0;
static double compass_heading = 0.0;

void compass_init(void) {
  buff_ready = false;

  uint8_t config[] = {
    0x15, // 32x average, 1-byte I2C read for 16 bit sensor data
    0x12, // low noise mode, continuous mode
    0x70, // XYX channels enabled
    0x00, // 40mT range
    0x00, // no threshold comparison for x
    0x00, // no threshold comparison for y
    0x00, // no threshold comparison for z
    0x00, // no temperature channel
    0xA4, // pulsed interrupt, enable interrupt through INT
  };

  if(HAL_I2C_Mem_Write(&hi2c1, COMPASS_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, config, sizeof(config), 100) != HAL_OK) {
    puts("compass init failed");
    return;
  }
}

void compass_run(void) {
  if(!buff_ready) return;

  double alpha = config_get(CONFIG_ENTRY_COMPASS_ALPHA);

  x = alpha * x + (1 - alpha) * (int16_t) ((i2c_buff[0] << 8) | i2c_buff[1]);
  y = alpha * y + (1 - alpha) * (int16_t) ((i2c_buff[2] << 8) | i2c_buff[3]);
  z = alpha * z + (1 - alpha) * (int16_t) ((i2c_buff[4] << 8) | i2c_buff[5]);

  buff_ready = false;

  switch(compass_state) {
    case COMPASS_STATE_RUN:
    {

      min_x = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      min_y = config_get(CONFIG_ENTRY_COMPASS_MIN_X);
      max_x = config_get(CONFIG_ENTRY_COMPASS_MAX_X);
      max_y = config_get(CONFIG_ENTRY_COMPASS_MAX_X);

      double x_norm = NORMALIZE(x, min_x, max_x);
      double y_norm = NORMALIZE(y, min_y, max_y);

      compass_heading = atan2(y_norm - 0.5, x_norm - 0.5);

      break;
    }

    case COMPASS_STATE_CALIBRATE:
      if(x < min_x) min_x = x;
      if(x > max_x) max_x = x;
      if(y < min_y) min_y = y;
      if(y > max_y) max_y = y;
      break;

    default:
      break;
  }
}

void compass_calibrate_start(void) {
  puts("starting calibration");
  min_x = min_y = INFINITY;
  max_x = max_y = -INFINITY;
  compass_state = COMPASS_STATE_CALIBRATE;
}

void compass_calibrate_end(void) {
  printf("min %5lf, %5lf; max %5lf, %5lf\n", min_x, min_y, max_x, max_y);
  if(min_x == max_x) max_x += 0.1;
  if(min_y == max_y) max_y += 0.1;

  config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_x);
  config_set(CONFIG_ENTRY_COMPASS_MIN_X, min_y);
  config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_x);
  config_set(CONFIG_ENTRY_COMPASS_MAX_X, max_y);

  compass_state = COMPASS_STATE_RUN;
}

double compass_getHeading(void) {
  return compass_heading;
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
  if(!buff_ready && pin == Mag_Trig_Pin) {
    HAL_I2C_Master_Receive_IT(&hi2c1, COMPASS_ADDR, i2c_buff, sizeof(i2c_buff));
  }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  buff_ready = true;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  buff_ready = true;
}
