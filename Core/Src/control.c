#include "control.h"
#include "motor.h"
#include "sensor.h"
#include "config.h"
#include "helper.h"
#include "pid.h"
#include "compass.h"
#include "usart.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

static control_state_E control_state = CONTROL_STATE_INIT;
static bool control_reset;

static pid_config_S pid_conf_sensor = {
  .kp = CONFIG_ENTRY_CTRL_KP,
  .ki = CONFIG_ENTRY_CTRL_KI,
  .kd = CONFIG_ENTRY_CTRL_KD,

  .output_max =  10.0,
  .output_min = -10.0,
};

static pid_config_S pid_conf_aim = {
  .kp = CONFIG_ENTRY_AIM_KP,
  .ki = CONFIG_ENTRY_AIM_KI,
  .kd = CONFIG_ENTRY_AIM_KD,

  .output_max =  2.0,
  .output_min = -2.0,
};

static pid_config_S pid_conf_count = {
  .kp = CONFIG_ENTRY_COUNT_KP,
  .ki = CONFIG_ENTRY_COUNT_KI,
  .kd = CONFIG_ENTRY_COUNT_KD,

  .output_max =  2.0,
  .output_min = -2.0,
};

static pid_data_S pid, pid2;
static double control_target;
static bool debug;

void control_init(void) {
  control_reset = true;
  debug = false;
  pid_init(&pid);
  pid_init(&pid2);
  control_setState(CONTROL_STATE_NEUTRAL);
}

void control_run(void) {
  switch(control_state) {
    case CONTROL_STATE_NEUTRAL:
      break;

    case CONTROL_STATE_SPEED:
      motor_setSpeed(M1, control_target);
      motor_setSpeed(M2, control_target);
      break;

    case CONTROL_STATE_FOLLOW:
    {
      double input = sensor_getResult();
      double u = pid_update(&pid, input, control_reset);

      if(u > 0) {
        motor_setSpeed(M1, control_target - u / 2);
        motor_setSpeed(M2, control_target + u / 2);
      } else {
        motor_setSpeed(M1, control_target - u / 2);
        motor_setSpeed(M2, control_target + u / 2);
      }

      break;
    }

    case CONTROL_STATE_HEADING:
    {
      double input = fmod(control_target - compass_getHeading() + M_PI, 2 * M_PI) - M_PI;
      double u = pid_update(&pid, input, control_reset);

      if(u > 0) {
        motor_setSpeed(M1, -u / 2);
        motor_setSpeed(M2,  u / 2);
      } else {
        motor_setSpeed(M1, -u / 2);
        motor_setSpeed(M2,  u / 2);
      }

      if(pid.stable) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    case CONTROL_STATE_MOVE:
    {
      double target = MOTOR_MM_TO_COUNT(control_target);
      double error1 = target - motor_getCount(M1);
      double error2 = target - motor_getCount(M2);

      double u1 = pid_update(&pid, error1, control_reset);
      double u2 = pid_update(&pid2, error2, control_reset);

      motor_setSpeed(M1, u1);
      motor_setSpeed(M2, u2);

      if(pid.stable && pid2.stable) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    case CONTROL_STATE_TURN:
    {
      double radius = config_get(CONFIG_ENTRY_WHEEL_DIST) / 2;
      double target = MOTOR_MM_TO_COUNT(radius * control_target / 180.0 * M_PI);
      double error1 = -target - motor_getCount(M1);
      double error2 =  target - motor_getCount(M2);

      double u1 = pid_update(&pid, error1, control_reset);
      double u2 = pid_update(&pid2, error2, control_reset);

      motor_setSpeed(M1, u1);
      motor_setSpeed(M2, u2);

      if(pid.stable && pid2.stable) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    default:
      break;
  }

  control_reset = false;

  if(debug) {
    printf("%ld %.4lf %.4f %.4lf %.4lf %.4lf\n",
      HAL_GetTick(),
      sensor_getResult(),
      sensor_getMean(),
      sensor_getVariance(),
      compass_getHeading(),
      sensor_getVBatt()
    );
  }
}

void control_setTarget(double target) {
  control_target = target;
}

void control_setState(control_state_E state) {
  // state transition
  if(state != control_state) {
    // state end action
    switch(control_state) {
      case CONTROL_STATE_HEADING:
        compass_setState(COMPASS_STATE_IDLE);
        break;

      default:
        break;
    }

    control_state = state;

    // state start action
    switch(control_state) {
      case CONTROL_STATE_NEUTRAL:
        motor_stop(M1);
        motor_stop(M2);
        break;

      case CONTROL_STATE_BRAKE:
        motor_setSpeed(M1, 0);
        motor_setSpeed(M2, 0);
        break;

      case CONTROL_STATE_FOLLOW:
        pid.config = pid_conf_sensor;
        control_reset = true;
        break;

      case CONTROL_STATE_HEADING:
        compass_setState(COMPASS_STATE_RUN);
        pid.config = pid_conf_aim;
        control_reset = true;
        break;

      case CONTROL_STATE_MOVE:
      case CONTROL_STATE_TURN:
        motor_resetCount(M1);
        motor_resetCount(M2);
        pid.config = pid_conf_count;
        pid2.config = pid_conf_count;
        control_reset = true;
        break;

      default:
        break;
    }
  }
}

control_state_E control_getState(void) {
  return control_state;
}

void control_debug(int8_t d) {
  if(d == 0)
    debug = false;
  else if(d == 1)
    debug = true;
  else if(d == -1)
    debug ^= 1;
}
