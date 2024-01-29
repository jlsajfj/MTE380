#include "control.h"
#include "motor.h"
#include "sensor.h"
#include "config.h"

#include <stdio.h>

typedef enum {
  CONTROL_STOP,
  CONTROL_RUNNING,
} control_state_E;

static control_state_E control_state = CONTROL_STOP;

void control_run(void) {
  switch(control_state) {
    case CONTROL_STOP:
        motor_setSpeed(M1, 0.0);
        motor_setSpeed(M2, 0.0);
      break;

    case CONTROL_RUNNING:
    {
      double kp = config_get(CONFIG_ENTRY_CTRL_KP);

      double input = sensor_getResult();
      double u = input * kp;

      if(u > 0) {
        motor_setSpeed(M1, 1.0 - u);
        motor_setSpeed(M2, 1.0);
      } else {
        motor_setSpeed(M1, 1.0);
        motor_setSpeed(M2, 1.0 + u);
      }

      break;
    }
  }
}

void control_start(void) {
  control_state = CONTROL_RUNNING;
}

void control_stop(void) {
  control_state = CONTROL_STOP;
}

void control_toggle(void) {
  switch(control_state) {
    case CONTROL_STOP:
      control_state = CONTROL_RUNNING;
      break;
    default:
    case CONTROL_RUNNING:
      control_state = CONTROL_STOP;
      break;
  }
}
