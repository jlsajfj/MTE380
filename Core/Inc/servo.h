#ifndef INC_SERVO_H_
#define INC_SERVO_H_

void servo_init(void);
void servo_run(void);
void servo_setPosition(double angle);  // takes value of 0 - 1 [revolutions]

#endif /* INC_SERVO_H_ */
