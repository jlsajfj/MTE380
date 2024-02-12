#ifndef INC_SERVO_H_
#define INC_SERVO_H_

void servo_init(void);

void servo_setPosition(double angle);  // takes value of 0 - 1 [revolutions]
void servo_setLocked(void);
void servo_setUnlocked(void);

#endif /* INC_SERVO_H_ */
