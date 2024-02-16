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
	MOTOR_NUM,
} motor_E;

void motor_init(void);
void motor_run(void);

void motor_move(motor_E motor_id, int32_t count);
void motor_setSpeed(motor_E motor_id, double speed);
void motor_stop(motor_E motor_id);

bool motor_getFault(void);
void motor_setEnabled(bool enabled);

#endif /* INC_MOTOR_H_ */
