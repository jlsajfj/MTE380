#include "command.h"
#include "usart.h"
#include "gpio.h"
#include "control.h"
#include "sensor.h"
#include "flash.h"
#include "servo.h"
#include "helper.h"
#include "config.h"
#include "motor.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

static char rx_char = 0;
static char rx_buff[64] = {0};
static uint16_t rx_idx = 0;

static volatile bool rx_pend = false;
static uint16_t rx_len = 0;
static bool echo = true;

static uint16_t btn_dbc = 0;

void command_init(void) {
  HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_char, 1);
}

void command_run(void) {
  uint32_t tick = HAL_GetTick();

  bool btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET;
  bool btn_rise = btn && (tick - btn_dbc) > 100;
  if(btn) btn_dbc = tick;

  if(btn_rise) {
    switch(control_getState()) {
      case CONTROL_STATE_STOP:
        //control_setState(CONTROL_STATE_RUN);
        control_setState(CONTROL_STATE_DEMO_1);
        break;

      default:
        control_setState(CONTROL_STATE_STOP);
        break;
    }
  }

#define MATCH_CMD(x) (strlen(x) == rx_len && strncmp((x), (const char*) rx_buff, rx_len) == 0)
#define MATCH_CMD_N(x, n) (strncmp((x), (const char*) rx_buff, MIN((n), rx_len)) == 0)

  if(rx_pend) {
    if(MATCH_CMD("start")) {
      control_setState(CONTROL_STATE_RUN);

    } else if(MATCH_CMD_N("aim ", 4)) {
      uint16_t heading_start = 3; while(isspace(rx_buff[heading_start]) && heading_start < rx_len) heading_start++;

      double heading = 0.0f;
      if(sscanf((const char*) (rx_buff + heading_start), "%lf", &heading) == 1) {
        control_aim(heading / 180.0 * M_PI);
      }

    } else if(MATCH_CMD("stop")) {
      control_setState(CONTROL_STATE_STOP);
      motor_stop(M1);
      motor_stop(M2);

    } else if(MATCH_CMD("calibrate")) {
      control_setState(CONTROL_STATE_CALIBRATE);

    } else if(MATCH_CMD("demo")) {
      control_setState(CONTROL_STATE_DEMO_1);

    } else if(MATCH_CMD_N("debug ", 6)) {
      uint16_t arg_start = 5; while(isspace(rx_buff[arg_start]) && arg_start < rx_len) arg_start++;

      if(strcmp("on", rx_buff + arg_start) == 0) {
        control_debug(1);
      } else if(strcmp("off", rx_buff + arg_start) == 0) {
        control_debug(0);
      } else {
        control_debug(-1);
      }

    } else if(MATCH_CMD_N("speed ", 6)) {
      uint16_t speed_start = 5; while(isspace(rx_buff[speed_start]) && speed_start < rx_len) speed_start++;

      double speed = 0.0f;
      if(sscanf((const char*) (rx_buff + speed_start), "%lf", &speed) == 1) {
        motor_setSpeed(M1, speed);
        motor_setSpeed(M2, speed);
      }

    } else if(MATCH_CMD_N("pwm ", 4)) {
      uint16_t pwm_start = 3; while(isspace(rx_buff[pwm_start]) && pwm_start < rx_len) pwm_start++;

      double pwm = 0.0f;
      if(sscanf((const char*) (rx_buff + pwm_start), "%lf", &pwm) == 1) {
        motor_setPWM(M1, pwm);
        motor_setPWM(M2, pwm);
      }

    } else if(MATCH_CMD_N("echo ", 5)) {
      uint16_t arg_start = 4; while(isspace(rx_buff[arg_start]) && arg_start < rx_len) arg_start++;

      if(strcmp("on", rx_buff + arg_start) == 0) {
        echo = true;
      } else if(strcmp("off", rx_buff + arg_start) == 0) {
        echo = false;
      } else {
        echo ^= 1;
      }

    } else if(MATCH_CMD_N("servo ", 6)) {
      uint16_t position_start = 5; while(isspace(rx_buff[position_start]) && position_start < rx_len) position_start++;

      double position = 0.0f;
      double valid = true;

      if(strcmp("lock", rx_buff + position_start) == 0) {
        position = config_get(CONFIG_ENTRY_SERVO_LOCK);
      } else if(strcmp("unlock", rx_buff + position_start) == 0) {
        position = config_get(CONFIG_ENTRY_SERVO_UNLOCK);
      } else if(sscanf((const char*) (rx_buff + position_start), "%lf", &position) != 1) {
        valid = false;
      }

      if(valid) {
        printf("set servo to %f\n", position);
        servo_setPosition(position);
      } else {
        puts("invalid value");
      }

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
      config_load();
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

      if(name_end == name_start) {
        config_print();
      } else {
        double value = config_getByName(rx_buff + name_start);
        printf("%16s = %lf\n", rx_buff + name_start, value);
      }
    } else {
      puts("unknown command");
    }

#undef MATCH_CMD

    rx_pend = false;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(echo) _write(0, &rx_char, 1);
  HAL_UART_Receive_IT(huart, (uint8_t*) &rx_char, 1);

  if(rx_char == '\b') {
    if(rx_idx > 0) {
      char erase[] = " \b";
      if(echo) _write(0, erase, 2);
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
