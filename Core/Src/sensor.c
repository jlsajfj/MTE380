#include "sensor.h"
#include "helper.h"
#include "adc.h"
#include "flash.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdio.h>

#define ADC_COUNT 6

typedef enum {
  ADC_STATUS_INVALID,
  ADC_STATUS_STARTED,
  ADC_STATUS_FINISHED,
} adc_status_E;

typedef enum {
  SENSOR_STATE_RUNNING,
  SENSOR_STATE_CAL_WHITE,
  SENSOR_STATE_CAL_BLACK,
} sensor_state_E;

typedef struct {
  double white[ADC_COUNT];
  double black[ADC_COUNT];
} sensor_calib_S;

typedef union {
  sensor_calib_S data;
  uint8_t raw[sizeof(sensor_calib_S)];
} sensor_calib_U;

static const double sensor_gains[ADC_COUNT] = {-3.5, -1.0, -0.5, 0.5, 1, 3.5};

static sensor_state_E sensor_state = SENSOR_STATE_RUNNING;

static volatile adc_status_E adc_status = ADC_STATUS_INVALID;
static uint16_t adc_reading_raw[ADC_COUNT] = {0};
static double adc_reading[ADC_COUNT] = {0.0};

static sensor_calib_U sensor_calib = {0};

static double sensor_result = 0.0;

void sensor_init(void) {
}

void sensor_run(void) {
  switch(adc_status) {
    case ADC_STATUS_FINISHED:
    {
      double alpha = config_get(CONFIG_ENTRY_SENSOR_ALPHA);
      for(uint16_t i = 0; i < ADC_COUNT; i++) {
        adc_reading[i] = alpha * adc_reading[i] + (1 - alpha) * adc_reading_raw[i];
      }

      switch(sensor_state) {
        case SENSOR_STATE_RUNNING:
          sensor_result = 0.0;
          for(uint16_t i = 0; i < ADC_COUNT; i++) {
            double normalized = NORMALIZE(adc_reading[i], sensor_calib.data.black[i], sensor_calib.data.white[i]);
            sensor_result += SATURATE(normalized, 0, 1) * sensor_gains[i];
          }
          break;

        case SENSOR_STATE_CAL_WHITE:
          for(uint16_t i = 0; i < ADC_COUNT; i++) {
              sensor_calib.data.white[i] = adc_reading[i];
              printf("white: %11.4f", adc_reading[i]);
          }
          puts("");
          sensor_state = SENSOR_STATE_RUNNING;
          break;

        case SENSOR_STATE_CAL_BLACK:
          for(uint16_t i = 0; i < ADC_COUNT; i++) {
              sensor_calib.data.black[i] = adc_reading[i];
              printf("black: %11.4f", adc_reading[i]);
          }
          puts("");
          sensor_state = SENSOR_STATE_RUNNING;
          break;
      }
    }

    // fallthrough
    case ADC_STATUS_INVALID:
      HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_reading_raw, ADC_COUNT);
      adc_status = ADC_STATUS_STARTED;
      break;

    default:
      break;
  }
}

double sensor_getResult(void) {
  return sensor_result;
}

bool sensor_valid(void) {
  return sensor_state == SENSOR_STATE_RUNNING;
}

void sensor_calibrate_white(void) {
  sensor_state = SENSOR_STATE_CAL_WHITE;
}

void sensor_calibrate_black(void) {
  sensor_state = SENSOR_STATE_CAL_BLACK;
}

void sensor_calibrate_load(void) {
  flash_read(FLASH_ADDR_SENSOR_CALIB, sensor_calib.raw, sizeof(sensor_calib));

  printf("white: ");
  for(uint16_t i = 0; i < ADC_COUNT; i++) {
    printf("% 11.4f", sensor_calib.data.white[i]);
  }
  printf("\nblack: ");
  for(uint16_t i = 0; i < ADC_COUNT; i++) {
    printf("% 11.4f", sensor_calib.data.black[i]);
  }
  puts("");
}

void sensor_calibrate_save(void) {
  flash_write(FLASH_ADDR_SENSOR_CALIB, sensor_calib.raw, sizeof(sensor_calib));
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  adc_status = ADC_STATUS_FINISHED;
}
