#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  CONTROL_STATE_INIT,
  CONTROL_STATE_NEUTRAL,
  CONTROL_STATE_BRAKE,
  CONTROL_STATE_FOLLOW,
  CONTROL_STATE_SPEED,
  CONTROL_STATE_HEADING,
  CONTROL_STATE_MOVE,
  CONTROL_STATE_TURN,
} control_state_E;

void control_init(void);
void control_run(void);

void control_setTarget(double target);
void control_setState(control_state_E state);
control_state_E control_getState(void);

void control_debug(int8_t);

#endif
