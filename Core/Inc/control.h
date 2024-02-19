#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdbool.h>


typedef enum {
  CONTROL_STATE_STOP,
  CONTROL_STATE_RUN,
  CONTROL_STATE_AIM,
  CONTROL_STATE_CALIBRATE,
  CONTROL_STATE_DEBUG,
} control_state_E;

void control_init(void);
void control_run(void);

void control_setState(control_state_E state);
control_state_E control_getState(void);

#endif
