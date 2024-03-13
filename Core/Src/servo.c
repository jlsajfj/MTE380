#include "servo.h"
#include "main.h"
#include "tim.h"
#include "config.h"
#include "helper.h"

#include "stm32f4xx_hal.h"

#define SERVO_PULSE_MIN 26214
#define SERVO_PULSE_MAX 107479
#define SERVO_RUN_TIME 1000

typedef struct {
  __IO uint32_t *pwm_reg;
  TIM_HandleTypeDef *pwm_timer;
  uint32_t pwm_timer_channel;
} servo_definition_S;

typedef struct {
  uint32_t start;
  uint32_t pulse;
} servo_data_S;

static servo_definition_S servos[SERVO_COUNT] = {
  [S1] = { &TIM2->CCR3, &htim2, TIM_CHANNEL_3 },
  [S2] = { &TIM2->CCR1, &htim2, TIM_CHANNEL_1 },
};

static servo_data_S servo_datas[SERVO_COUNT];

void servo_init(void) {
  for(servo_E servo_id = S1; servo_id < SERVO_COUNT; servo_id++) {
    const servo_definition_S *servo = &servos[servo_id];
    servo_data_S *data = &servo_datas[servo_id];
    data->pulse = SERVO_PULSE_MIN;
    data->start = HAL_GetTick() - SERVO_RUN_TIME;
    *servo->pwm_reg = SERVO_PULSE_MIN;
    HAL_TIM_PWM_Start(servo->pwm_timer, servo->pwm_timer_channel);
  }
}

void servo_run(void) {
  uint32_t tick = HAL_GetTick();
  for(servo_E servo_id = S1; servo_id < SERVO_COUNT; servo_id++) {
    const servo_definition_S *servo = &servos[servo_id];
    servo_data_S *data = &servo_datas[servo_id];

    // only power servo for a certain period of time
    if(tick - data->start >= SERVO_RUN_TIME) {
      *servo->pwm_reg = 0;
    } else {
      *servo->pwm_reg = data->pulse;
    }
  }
}

void servo_setPosition(servo_E servo_id, double angle) {
  servo_data_S *data = &servo_datas[servo_id];
  data->pulse = (uint32_t) MAP(SATURATE(angle, 0, 1), SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  data->start = HAL_GetTick();
}
