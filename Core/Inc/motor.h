#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include <stdbool.h>
#include <stdint.h>

// gear ratio * encoder count (rising edge) / diameter / pi
#define MOTOR_COUNT_PER_MM (10 * 30 / 29.0 / 3.14159)
#define MOTOR_MM_TO_COUNT(x) ((int32_t) (x) * MOTOR_COUNT_PER_MM)

typedef enum {
	M1,
	M2,
	MOTOR_COUNT,
} motor_E;

void motor_init(void);
void motor_run(void);

void motor_setSpeed(motor_E motor_id, double speed);
void motor_setPWM(motor_E motor_id, double pwm);
void motor_stop(motor_E motor_id);

int32_t motor_getCount(motor_E motor_id);
double motor_getSpeed(motor_E motor_id);

#endif /* INC_MOTOR_H_ */
