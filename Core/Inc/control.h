#ifndef __CONTROL_H__
#define __CONTROL_H__

typedef enum {
	CONTROL_GAIN_P,
} control_gain_E;

void control_run(void);

void control_setGain(control_gain_E gain_type, double gain);

void control_start(void);
void control_stop(void);
void control_toggle(void);

#endif
