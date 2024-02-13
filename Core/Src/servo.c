#include "servo.h"
#include "main.h"
#include "tim.h"
#include "config.h"
#include "helper.h"

#include "stm32f4xx_hal.h"

#define SERVO_PWM_MIN 20971
#define SERVO_PWM_MAX 117964
#define SERVO_RUN_TIME 1000

typedef struct {
  __IO uint32_t *pwm_reg;
  TIM_HandleTypeDef *pwm_timer;
  uint32_t pwm_timer_channel;
} servo_definition_S;

static servo_definition_S servo = { &TIM2->CCR3, &htim2, TIM_CHANNEL_3 };
static uint32_t servo_pwm;
static uint32_t servo_start;

void servo_init(void) {
  servo_pwm = SERVO_PWM_MIN;
  servo_start = HAL_GetTick() - SERVO_RUN_TIME;
  *servo.pwm_reg = SERVO_PWM_MIN;
  HAL_TIM_PWM_Start(servo.pwm_timer, servo.pwm_timer_channel);
}

void servo_run(void) {
  // only power servo for a certain period of time
  uint32_t tick = HAL_GetTick();
  if(tick - servo_start >= SERVO_RUN_TIME) {
    *servo.pwm_reg = 0;
  } else {
    *servo.pwm_reg = servo_pwm;
  }
}

void servo_setPosition(double angle) {
  servo_pwm = (uint32_t) MAP(SATURATE(angle, 0, 1), SERVO_PWM_MIN, SERVO_PWM_MAX);
  servo_start = HAL_GetTick();
}
