#include "servo.h"
#include "main.h"
#include "tim.h"
#include "config.h"
#include "helper.h"
#include "main.h"

#include "stm32f4xx_hal.h"

#define SERVO_PULSE_MIN 26214
#define SERVO_PULSE_MAX 107479
#define SERVO_RUN_TIME 1000

typedef struct {
  __IO uint32_t *pwm_reg;
  TIM_HandleTypeDef *pwm_timer;
  uint32_t pwm_timer_channel;
  bool sw_pwm;
} servo_definition_S;

typedef struct {
  uint32_t start;
  uint32_t pulse;
  bool locked;
} servo_data_S;

static servo_definition_S servos[SERVO_COUNT] = {
  [S1] = { &TIM2->CCR3, &htim2, TIM_CHANNEL_3, false },
  [S2] = {        NULL,   NULL,             0, true  },
};

static servo_data_S servo_datas[SERVO_COUNT];
static uint32_t servo_tick;

void servo_setPositionPrivate(servo_E servo_id, double angle);

void servo_init(void) {
  servo_tick = 0;

  for(servo_E servo_id = S1; servo_id < SERVO_COUNT; servo_id++) {
    const servo_definition_S *servo = &servos[servo_id];
    servo_data_S *data = &servo_datas[servo_id];
    data->pulse = SERVO_PULSE_MIN;
    data->start = 0;
    if(!servo->sw_pwm) {
      *servo->pwm_reg = SERVO_PULSE_MIN;
      HAL_TIM_PWM_Start(servo->pwm_timer, servo->pwm_timer_channel);
    }
  }
}

void servo_run(void) {
  for(servo_E servo_id = S1; servo_id < SERVO_COUNT; servo_id++) {
    const servo_definition_S *servo = &servos[servo_id];
    servo_data_S *data = &servo_datas[servo_id];

    // only power servo for a certain period of time
    if(servo_tick - data->start >= SERVO_RUN_TIME) {
      if(servo->sw_pwm) {
        HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, GPIO_PIN_RESET);
      } else {
        *servo->pwm_reg = 0;
      }
    } else {
      if(servo->sw_pwm) {
        uint16_t pw = (data->pulse >= (SERVO_PULSE_MAX + SERVO_PULSE_MIN) / 2) ? 2 : 1;
        GPIO_PinState out = (servo_tick % (2 * pw) >= pw) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(Servo2_GPIO_Port, Servo2_Pin, out);
      } else {
        *servo->pwm_reg = data->pulse;
      }
    }
  }

  servo_tick++;
}

void servo_setPosition(servo_E servo_id, double angle) {
  if(servo_id >= SERVO_COUNT) return;
  servo_setPositionPrivate(servo_id, angle);
  servo_data_S *data = &servo_datas[servo_id];
  data->locked = false;
}

void servo_setPositionPrivate(servo_E servo_id, double angle) {
  if(servo_id >= SERVO_COUNT) return;
  servo_data_S *data = &servo_datas[servo_id];
  data->pulse = (uint32_t) MAP(SATURATE(angle, 0, 1), SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  data->start = servo_tick;
}

void servo_lock(servo_E servo_id, bool lock) {
  if(servo_id >= SERVO_COUNT) return;
  servo_data_S *data = &servo_datas[servo_id];

  if(lock) {
    servo_setPosition(servo_id, config_get(CONFIG_ENTRY_SERVO_LOCK));
  } else {
    servo_setPosition(servo_id, config_get(CONFIG_ENTRY_SERVO_UNLOCK));
  }

  data->locked = lock;
}

bool servo_isLocked(servo_E servo_id) {
  if(servo_id >= SERVO_COUNT) return false;
  servo_data_S *data = &servo_datas[servo_id];
  return data->locked;
}
