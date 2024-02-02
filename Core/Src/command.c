#include "command.h"
#include "usart.h"
#include "gpio.h"
#include "control.h"
#include "sensor.h"
#include "flash.h"
#include "helper.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char rx_char = 0;
static char rx_buff[64] = {0};
static uint16_t rx_idx = 0;

static volatile bool rx_pend = false;
static uint16_t rx_len = 0;

static uint16_t btn_dbc = 0;

void command_init(void) {
  HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_char, 1);
}

void command_run(void) {
  uint32_t tick = HAL_GetTick();

  bool btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET;
  bool btn_rise = btn && (tick - btn_dbc) > 200;
  if(btn) btn_dbc = tick;

  if(btn_rise) {
    control_toggle();
  }

#define MATCH_CMD(x) (strncmp((x), (const char*) rx_buff, rx_len) == 0)
#define MATCH_CMD_N(x, n) (strncmp((x), (const char*) rx_buff, MIN((n), rx_len)) == 0)

  if(rx_pend) {
    if(MATCH_CMD("start")) {
      control_start();
    } else if(MATCH_CMD("stop")) {
      control_stop();
    } else if(MATCH_CMD("debug")) {
      control_debug();
    } else if(MATCH_CMD("white")) {
      sensor_calibrate_white();
    } else if(MATCH_CMD("black")) {
      sensor_calibrate_black();

    } else if(MATCH_CMD("save")) {
      flash_erase();
      config_save();
      puts("saved");

    } else if(MATCH_CMD("load")) {
      config_load();

    } else if(MATCH_CMD("erase")) {
      flash_erase();
      puts("erased");

    } else if(MATCH_CMD("reset")) {
      NVIC_SystemReset();

    } else if(MATCH_CMD_N("set ", 4)) {
      uint16_t name_start = 3; while(isspace(rx_buff[name_start]) && name_start < rx_len) name_start++;
      uint16_t name_end = name_start; while(!isspace(rx_buff[name_end]) && name_end < rx_len) name_end++;
      uint16_t value_start = name_end; while(isspace(rx_buff[value_start]) && value_start < rx_len) value_start++;

      rx_buff[name_end] = '\0';

      double value = 0.0;
      if(sscanf((const char*) (rx_buff + value_start), "%lf", &value) == 1) {
        config_setByName(rx_buff + name_start, value);
      }

      value = config_getByName(rx_buff + name_start);
      printf("%16s = %lf\n", rx_buff + name_start, value);

    } else if(MATCH_CMD_N("get ", 4)) {
      uint16_t name_start = 3; while(isspace(rx_buff[name_start]) && name_start < rx_len) name_start++;
      uint16_t name_end = name_start; while(!isspace(rx_buff[name_end]) && name_end < rx_len) name_end++;
      rx_buff[name_end] = '\0';

      double value = config_getByName(rx_buff + name_start);

      printf("%16s = %lf\n", rx_buff + name_start, value);
    } else {
      puts("start, stop, debug, white, black, save, load, reset, boot, set, get");
    }

#undef MATCH_CMD

    rx_pend = false;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  HAL_UART_Transmit(huart, (uint8_t*) &rx_char, 1, 100);
  HAL_UART_Receive_IT(huart, (uint8_t*) &rx_char, 1);

  if(rx_char == '\b') {
    if(rx_idx > 0) {
      uint8_t erase[] = " \b";
      HAL_UART_Transmit(huart, erase, 2, 100);
      rx_idx--;
    }
  } else if(rx_char == '\r' || rx_char == '\n') {
    if(rx_idx > 0) {
      rx_buff[rx_idx] = '\0';
      rx_len = rx_idx;
      rx_idx = 0;
      rx_pend = true;
    }
  } else if(rx_idx < sizeof(rx_buff)) {
    rx_buff[rx_idx++] = rx_char;
  }
}
