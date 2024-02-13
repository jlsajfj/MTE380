#include "control.h"
#include "motor.h"
#include "sensor.h"
#include "config.h"
#include "helper.h"
#include "pid.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <stdbool.h>

typedef enum {
  CONTROL_STOP,
  CONTROL_RUNNING,
  CONTROL_TURNING_START,
  CONTROL_TURNING_FIND_LINE,
  CONTROL_DEBUG,
} control_state_E;

static control_state_E control_state = CONTROL_STOP;

static pid_data_S pid = {
   .kp_config = CONFIG_ENTRY_CTRL_KP,
   .ki_config = CONFIG_ENTRY_CTRL_KI,
   .kd_config = CONFIG_ENTRY_CTRL_KD,

   .output_max =  2.0,
   .output_min = -2.0,
};

void control_init(void) {
  pid_init(&pid);
}

void control_run(void) {
  switch(control_state) {
    case CONTROL_STOP:
        motor_setSpeed(M1, 0.0);
        motor_setSpeed(M2, 0.0);
      break;

    case CONTROL_RUNNING:
    {
      double input = sensor_getResult();
      double u = pid_update(&pid, input);

      double speed = config_get(CONFIG_ENTRY_MOTOR_SPEED);
      speed -= config_get(CONFIG_ENTRY_CTRL_KS) * ABS(u);
      speed = SATURATE(speed, -1.0 + u/2, 1.0 - u/2);

      if(u > 0) {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      } else {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      }

      break;
    }

    case CONTROL_TURNING_START:
    {
      double speed = config_get(CONFIG_ENTRY_MOTOR_SPEED);
      motor_setSpeed(M1, -speed);
      motor_setSpeed(M2, speed);

      double input = sensor_getAverage();
      if(ABS(input) > 1.0) {
        control_state = CONTROL_TURNING_FIND_LINE;
      }

      break;
    }

    case CONTROL_TURNING_FIND_LINE:
    {
      double speed = config_get(CONFIG_ENTRY_MOTOR_SPEED);
      motor_setSpeed(M1, -speed);
      motor_setSpeed(M2, speed);

      double input = sensor_getAverage();
      if(ABS(input) < 1.0) {
        control_state = CONTROL_STOP;
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

void control_turnAround(void) {
  control_state = CONTROL_TURNING_START;
}
