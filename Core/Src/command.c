#include "command.h"
#include "usart.h"
#include "control.h"
#include "sensor.h"
#include "flash.h"
#include "servo.h"
#include "helper.h"
#include "config.h"
#include "motor.h"
#include "state.h"
#include "telemetry.h"
#include "speed.h"
#include "music.h"

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

void command_init(void) {
  HAL_UART_Receive_IT(&huart2, (uint8_t*) &rx_char, 1);
}

void command_run(void) {
  char sarg[64] = {0};
  double darg;
  bool command_success = true;

#define MATCH_CMD(x) (strlen(x) == rx_len && strncmp((x), (const char*) rx_buff, rx_len) == 0)
#define MATCH_CMD_S(x) (sscanf((const char*) rx_buff, x " %s", sarg) == 1)
#define MATCH_CMD_D(x) (sscanf((const char*) rx_buff, x " %lf", &darg) == 1)
#define MATCH_CMD_SD(x) (sscanf((const char*) rx_buff, x " %s %lf", sarg, &darg) == 2)

  if(rx_pend) {
    if(MATCH_CMD("start")) {
      sm_setState(SM_STATE_UNHOOK);

    } else if(MATCH_CMD("stop")) {
      sm_setState(SM_STATE_STANDBY);
      control_setState(CONTROL_STATE_NEUTRAL);
      music_stop();
      motor_stop(M1);
      motor_stop(M2);

    } else if(MATCH_CMD("align")) {
      sm_setState(SM_STATE_ALIGN);

    } else if(MATCH_CMD("calibrate")) {
      sm_setState(SM_STATE_CALIBRATE);

    } else if(MATCH_CMD("record")) {
      sm_setState(SM_STATE_RECORD);

    } else if(MATCH_CMD("white")) {
      sensor_calibrate_white();

    } else if(MATCH_CMD("black")) {
      sensor_calibrate_black();

    } else if(MATCH_CMD_D("aim")) {
      control_setTarget(darg);
      control_setState(CONTROL_STATE_HEADING);

    } else if(MATCH_CMD_D("speed")) {
      control_setTarget(darg);
      control_setState(CONTROL_STATE_SPEED);

    } else if(MATCH_CMD_D("move")) {
      control_setTarget(darg);
      control_setState(CONTROL_STATE_MOVE);

    } else if(MATCH_CMD_D("turn")) {
      control_setTarget(darg);
      control_setState(CONTROL_STATE_TURN);

    } else if(MATCH_CMD_D("arc")) {
      control_setTarget(darg);
      control_setState(CONTROL_STATE_ARC);

    } else if(MATCH_CMD_D("pwm")) {
      control_setState(CONTROL_STATE_NEUTRAL);
      motor_setPWM(M1, darg);
      motor_setPWM(M2, darg);

    } else if(MATCH_CMD_D("state")) {
      int16_t state = (int16_t) darg;
      if(darg >= 0 && darg < SM_STATE_COUNT) {
        sm_setState(state);
      }

    } else if(MATCH_CMD_D("kick")) {
      servo_setPosition(S1, darg);

    } else if(MATCH_CMD_S("kick")) {
      double position = 0;

      if(sarg[0] == 'l') {
        position = config_get(CONFIG_ENTRY_SERVO_LOCK);
      } else {
        position = config_get(CONFIG_ENTRY_SERVO_UNLOCK);
      }

      servo_setPosition(S1, position);

    } else if(MATCH_CMD_D("hook")) {
      servo_setPosition(S2, darg);

    } else if(MATCH_CMD_S("hook")) {
      double position = 0;

      if(sarg[0] == 'l') {
        position = config_get(CONFIG_ENTRY_SERVO_LOCK);
      } else {
        position = config_get(CONFIG_ENTRY_SERVO_UNLOCK);
      }

      servo_setPosition(S2, position);

    } else if(MATCH_CMD("save")) {
      flash_erase();
      speed_save();
      config_save();
      puts("saved");

    } else if(MATCH_CMD("load")) {
      speed_load();
      config_load();
      tele_dumpConfig();

    } else if(MATCH_CMD("erase")) {
      flash_erase();
      speed_load();
      config_load();
      puts("erased");
      tele_dumpConfig();

    } else if(MATCH_CMD("reset")) {
      NVIC_SystemReset();

    } else if(MATCH_CMD("reset_speed")) {
      speed_reset();
      puts("speed points cleared");

    } else if(MATCH_CMD_SD("set")) {
      config_setByName(sarg, darg);
      double value = config_getByName(sarg);
      printf("%16s = %lf\n", sarg, value);
      tele_dumpConfig();

    } else if(MATCH_CMD("get")) {
      config_print();
      tele_dumpConfig();

    } else if(MATCH_CMD_S("get")) {
      double value = config_getByName(sarg);
      printf("%16s = %lf\n", sarg, value);

    } else if(MATCH_CMD("debug")) {
      control_debug(-1);

    } else if(MATCH_CMD_D("debug")) {
      control_debug((int) darg);

    } else if(MATCH_CMD_D("stream")) {
      tele_setEnabled(darg != 0);
      uart_setTxFD(darg == 0 ? 1 : 2);

    } else if(MATCH_CMD("sync")) {
      tele_sync();

    } else if(MATCH_CMD("batt")) {
      double vbatt = sensor_getVBatt();
      printf("batt: %10.2lf\n", vbatt);

    } else {
      puts("unknown command");
      command_success = false;
    }

    tele_respond(command_success);

    rx_pend = false;
  }

#undef MATCH_CMD
#undef MATCH_CMD_S
#undef MATCH_CMD_D
#undef MATCH_CMD_SD

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  _write(1, &rx_char, 1);
  HAL_UART_Receive_IT(huart, (uint8_t*) &rx_char, 1);

  if(rx_char == '\b') {
    if(rx_idx > 0) {
      char erase[] = " \b";
      _write(1, erase, 2);
      rx_idx--;
    }
  } else if(rx_char == '\r' || rx_char == '\n') {
    if(!rx_pend && rx_idx > 0) {
      rx_buff[rx_idx] = '\0';
      rx_len = rx_idx;
      rx_idx = 0;
      rx_pend = true;
    }
  } else if(rx_idx < sizeof(rx_buff)) {
    rx_buff[rx_idx++] = rx_char;
  }
}
