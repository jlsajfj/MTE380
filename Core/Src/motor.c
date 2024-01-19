/*
 * DualMAX14870MotorShield.c
 *
 *  Created on: Jan 19, 2024
 *      Author: croni
 */
#include "motor.h"
#include "tim.h"
#include "stm32f4xx_hal.h"

static bool motor_initialized = 0;
static bool motor_flip[MOTOR_NUM] = { false, false };

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	__IO uint32_t *pwm_reg;
	TIM_HandleTypeDef *pwm_timer;
	uint32_t pwm_timer_channel;
} motor_definition_S;

static motor_definition_S motors[MOTOR_NUM] = {
	[M1] = { GPIOA, GPIO_PIN_8, &TIM3->CCR2, &htim3, TIM_CHANNEL_2 },
	[M2] = { GPIOA, GPIO_PIN_9, &TIM4->CCR1, &htim4, TIM_CHANNEL_1 },
};

void motor_init(void) {
	for(motor_E motor = M1; motor < MOTOR_NUM; motor++) {
		motor_definition_S *motor_def = &motors[motor];
		HAL_TIM_PWM_Start(motor_def->pwm_timer, motor_def->pwm_timer_channel);
	}
	motor_initialized = true;
}

void motor_setSpeed(motor_E motor, int16_t speed) {
	if(!motor_initialized) return;

	const motor_definition_S *motor_def = &motors[motor];
	const bool reversed = (speed < 0) != motor_flip[motor];
	if(speed < 0) speed = -speed;

	HAL_GPIO_WritePin(motor_def->port, motor_def->pin, reversed ? GPIO_PIN_SET : GPIO_PIN_RESET);

	// TODO verify writing to register via pointer works
	*motor_def->pwm_reg = (int)(((float)speed/(1<<15))*100); // currently setup for the PWM to take a number from 0 - 100
}

void motor_setFlip(motor_E motor, bool flip) {
	motor_flip[motor] = flip;
}

bool motor_getFault(void) {
	return false;
}

void motor_setEnabled(bool) {
}
