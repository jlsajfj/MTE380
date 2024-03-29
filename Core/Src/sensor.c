#include "sensor.h"
#include "helper.h"
#include "adc.h"
#include "flash.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdio.h>

#define ADC_COUNT 7

#define VBATT_INDEX SENSOR_PD_COUNT
#define VBATT_COEFF ((4.7 + 2.2) / 2.2 * 3.3 / (1<<12)) // voltage divider * vref / 12bit

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

static sensor_state_E sensor_state = SENSOR_STATE_RUNNING;

static volatile adc_status_E adc_status = ADC_STATUS_INVALID;
static uint16_t adc_reading_raw[ADC_COUNT] = {0};
static double adc_reading[ADC_COUNT] = {0.0};

static double sensor_result = 0.0;
static double sensor_mean = 0.0;
static double sensor_variance = 0.0;
static double sensor_values[SENSOR_PD_COUNT] = {0.0};

void sensor_init(void) {
}

void sensor_run(void) {
  switch(adc_status) {
    case ADC_STATUS_FINISHED:
    {
      double alpha = config_get(CONFIG_ENTRY_SENSOR_ALPHA);
      for(uint16_t i = 0; i < ADC_COUNT; i++) {
        adc_reading[i] = alpha * adc_reading[i] + (1 - alpha) * adc_reading_raw[i];
        //printf("%11.4f", adc_reading[i]);
      }
      //puts("");

      switch(sensor_state) {
        case SENSOR_STATE_RUNNING:
        {
          double result = 0.0;
          double sum = 0.0;
          double ssum = 0.0;

          for(uint16_t i = 0; i < SENSOR_PD_COUNT; i++) {
            double white = config_get(CONFIG_ENTRY_SENSOR_WHITE_0 + i);
            double black = config_get(CONFIG_ENTRY_SENSOR_BLACK_0 + i);

            double gain = 1.0;
            if(i <= 2) {
              gain = -config_get(CONFIG_ENTRY_SENSOR_GAIN_2 - i);
            } else {
              gain = config_get(CONFIG_ENTRY_SENSOR_GAIN_0 - 3 + i);
            }

            sensor_values[i] = NORMALIZE(adc_reading[i], black, white);

            sum += sensor_values[i];
            ssum += sensor_values[i] * sensor_values[i];
            result += SATURATE(sensor_values[i], 0, 1) * gain;

            //printf("%11.4f", sensor_values[i]);
          }
          //puts("");

          sensor_result = result;
          sensor_mean = sum / SENSOR_PD_COUNT;
          sensor_variance = ssum / SENSOR_PD_COUNT - sensor_mean * sensor_mean;

          break;
        }

        case SENSOR_STATE_CAL_WHITE:
          puts("white: ");
          for(uint16_t i = 0; i < SENSOR_PD_COUNT; i++) {
              config_set(CONFIG_ENTRY_SENSOR_WHITE_0 + i, adc_reading[i]);
              printf("%11.4f", adc_reading[i]);
          }
          puts("");
          sensor_state = SENSOR_STATE_RUNNING;
          break;

        case SENSOR_STATE_CAL_BLACK:
          puts("black: ");
          for(uint16_t i = 0; i < SENSOR_PD_COUNT; i++) {
              config_set(CONFIG_ENTRY_SENSOR_BLACK_0 + i, adc_reading[i]);
              printf("%11.4f", adc_reading[i]);
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

double sensor_getMean(void) {
  return sensor_mean;
}

double sensor_getVariance(void) {
  return sensor_variance;
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

double sensor_getVBatt(void) {
  return adc_reading[VBATT_INDEX] * VBATT_COEFF;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  adc_status = ADC_STATUS_FINISHED;
}

double sensor_getValue(uint32_t index) {
  if(index < SENSOR_PD_COUNT) {
    return sensor_values[index];
  }
  return 0.0;
}
