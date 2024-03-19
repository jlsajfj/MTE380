#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

// gear ratio * count per turn / diameter / pi
#define MOTOR_COUNT_PER_MM (30 * 20 / 28.0 / M_PI)
#define MOTOR_MM_TO_COUNT(x) ((int32_t) round((x) * MOTOR_COUNT_PER_MM))

typedef enum {
	M1,
	M2,
	MOTOR_COUNT,
} motor_E;

void motor_init(void);
void motor_run(void);
void motor_timerIT(TIM_HandleTypeDef *htim);

void motor_setSpeed(motor_E motor_id, double speed);
void motor_setPWM(motor_E motor_id, double pwm);
void motor_stop(motor_E motor_id);
void motor_buzz(motor_E motor_id, double freq);

void motor_resetCount(motor_E motor_id);
int32_t motor_getCount(motor_E motor_id);
double motor_getSpeed(motor_E motor_id);
double motor_getSpeedTarget(motor_E motor_id);
bool motor_isStable(motor_E motor_id);

#endif /* INC_MOTOR_H_ */
