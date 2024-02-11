#include "servo.h"
#include "main.h"
#include "tim.h"

#include "stm32f4xx_hal.h"

typedef struct {
	__IO uint32_t *pwm_reg;
	TIM_HandleTypeDef *pwm_timer;
	uint32_t pwm_timer_channel;
} servo_definition_S;

static servo_definition_S servo = { &TIM2->CCR3, &htim2, TIM_CHANNEL_3 };




void servo_init(void) {
	HAL_TIM_PWM_Start(&servo->pwm_timer, &servo->pwm_timer_channel)
	&servo->pwm_reg = (uint32_t)(servo_closed * 10000);  // need magic param for servo_closed, should be controlled by command
}

void servo_setPosition(double angle) {
	&servo->pwm_reg = (uint32_t)(angle * 10000);
}
