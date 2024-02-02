#include "control.h"
#include "motor.h"
#include "sensor.h"
#include "config.h"

#include <stdio.h>

typedef enum {
  CONTROL_STOP,
  CONTROL_RUNNING,
  CONTROL_DEBUG,
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

      double speed = config_get(CONFIG_ENTRY_MOTOR_SPEED);

      if(u > 0) {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      } else {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      }

      break;
    }

    case CONTROL_DEBUG:
      printf("%lf\n", sensor_getResult());
      break;
  }
}

void control_start(void) {
  control_state = CONTROL_RUNNING;
}

void control_stop(void) {
  control_state = CONTROL_STOP;
}

void control_debug(void) {
  control_state = CONTROL_DEBUG;
}

void control_toggle(void) {
  switch(control_state) {
    case CONTROL_STOP:
      control_state = CONTROL_RUNNING;
      break;
    case CONTROL_RUNNING:
    default:
      control_state = CONTROL_STOP;
      break;
  }
}
