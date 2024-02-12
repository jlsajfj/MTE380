#include "servo.h"
#include "main.h"
#include "tim.h"
#include "config.h"

#include "stm32f4xx_hal.h"

typedef struct {
	__IO uint32_t *pwm_reg;
	TIM_HandleTypeDef *pwm_timer;
	uint32_t pwm_timer_channel;
} servo_definition_S;

static servo_definition_S servo = { &TIM2->CCR3, &htim2, TIM_CHANNEL_3 };




void servo_init(void) {
	HAL_TIM_PWM_Start(servo.pwm_timer, servo.pwm_timer_channel);
	double servo_position = config_get(CONFIG_ENTRY_SERVO_UNLOCK);
	*servo.pwm_reg = (uint32_t)(servo_position * 10000);  // need magic param for servo_locked, should be controlled by command
}

void servo_setPosition(double angle) {
	*servo.pwm_reg = (uint32_t)(angle * 10000);
}


void servo_setLocked(void) {
	double servo_position = config_get(CONFIG_ENTRY_SERVO_LOCK);
	*servo.pwm_reg = (uint32_t)(servo_position * 10000);
}

void servo_setUnlocked(void) {
	double servo_position = config_get(CONFIG_ENTRY_SERVO_UNLOCK);
	*servo.pwm_reg = (uint32_t)(servo_position * 10000);
}
