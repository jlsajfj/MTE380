/*
 * DualMAX14870MotorShield.c
 *
 *  Created on: Jan 19, 2024
 *      Author: croni
 */
#include "motor.h"
#include "main.h"
#include "tim.h"
#include "stm32f4xx_hal.h"

static bool motor_initialized = 0;
static bool motor_flip[MOTOR_NUM] = { false, false };

typedef struct {
	GPIO_TypeDef *dir_port;
	uint16_t dir_pin;
	__IO uint32_t *pwm_reg;
	TIM_HandleTypeDef *pwm_timer;
	uint32_t pwm_timer_channel;
} motor_definition_S;

static motor_definition_S motors[MOTOR_NUM] = {
	[M1] = { MtrDvr_Dir1_GPIO_Port, MtrDvr_Dir1_Pin, &TIM3->CCR2, &htim3, TIM_CHANNEL_2 },
	[M2] = { MtrDvr_Dir2_GPIO_Port, MtrDvr_Dir2_Pin, &TIM4->CCR1, &htim4, TIM_CHANNEL_1 },
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

	HAL_GPIO_WritePin(motor_def->dir_port, motor_def->dir_pin, reversed ? GPIO_PIN_SET : GPIO_PIN_RESET);

	// currently setup for the PWM to take a number from 0 - 100
	*motor_def->pwm_reg = (int)(((float)speed/(1<<15))*100);
}

void motor_setFlip(motor_E motor, bool flip) {
	motor_flip[motor] = flip;
}

bool motor_getFault(void) {
	return false;
}

void motor_setEnabled(bool enabled) {
}
