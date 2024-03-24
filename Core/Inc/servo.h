#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "stdbool.h"

typedef enum {
	S1,
	S2,
	SERVO_COUNT,
} servo_E;

void servo_init(void);
void servo_run(void);
void servo_setPosition(servo_E servo_id, double angle);  // takes value of 0 - 1 [revolutions]
void servo_lock(servo_E servo_id, bool lock);
bool servo_isLocked(servo_E servo_id);

#endif /* INC_SERVO_H_ */
