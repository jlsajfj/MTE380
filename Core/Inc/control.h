#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdbool.h>

typedef enum {
	CONTROL_GAIN_P,
} control_gain_E;

void control_init(void);
void control_run(void);

void control_setGain(control_gain_E gain_type, double gain);

void control_start(void);
void control_stop(void);
void control_toggle(void);
void control_debug(void);
void control_servo_mode(bool servo_mode);

#endif
