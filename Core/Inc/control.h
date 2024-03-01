#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  CONTROL_STATE_STOP,
  CONTROL_STATE_RUN,
  CONTROL_STATE_AIM,
  CONTROL_STATE_CALIBRATE,
} control_state_E;

void control_init(void);
void control_run(void);

void control_setState(control_state_E state);
control_state_E control_getState(void);
void control_debug(int8_t);

#endif
