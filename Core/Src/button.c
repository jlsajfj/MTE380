#include "button.h"
#include "gpio.h"
#include "servo.h"
#include "state.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdbool.h>

#define DEBOUNCE 50
#define LONG_PRESS 500

static uint16_t btn_down_count = 0;
static uint16_t btn_up_count = 0;

static bool servo_locked = true;

void button_short(void);
void button_long(void);

void button_init(void) {
}

void button_run(void) {
  bool btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET;

  if(btn) {
    if(btn_down_count == DEBOUNCE) {
      btn_up_count = 0;
    }

    if(btn_down_count <= LONG_PRESS) {
      btn_down_count++;
    }

  } else {
    if(btn_up_count == DEBOUNCE) {
      if(btn_down_count < LONG_PRESS) {
        button_short();
      } else {
        button_long();
      }

      btn_down_count = 0;
    }
    if(btn_up_count <= LONG_PRESS) {
      btn_up_count++;
    }
  }

  servo_locked = servo_isLocked(S1) && servo_isLocked(S2);
}

void button_short(void) {
  if(sm_getState() != SM_STATE_STANDBY) {
    sm_setState(SM_STATE_STANDBY);
  } else {
    servo_lock(S1, !servo_locked);
    servo_lock(S2, !servo_locked);
  }
}

void button_long(void) {
  if(servo_locked) {
    sm_setState(SM_STATE_UNHOOK);
  } else {
    sm_setState(SM_STATE_RECORD);
  }
}
