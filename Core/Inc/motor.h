#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	M1,
	M2,
	MOTOR_NUM,
} motor_E;

void motor_init(void);
void motor_run(void);

void motor_setSpeed(motor_E motor, double speed);
void motor_setFlip(motor_E motor, bool flip);
bool motor_getFault(void);
void motor_setEnabled(bool enabled);


#endif /* INC_MOTOR_H_ */
