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
#include <stdlib.h>

static control_state_E control_state = CONTROL_STATE_INIT;
static bool control_reset;

static pid_config_S pid_conf_sensor = {
  .kp = CONFIG_ENTRY_CTRL_KP,
  .ki = CONFIG_ENTRY_CTRL_KI,
  .kd = CONFIG_ENTRY_CTRL_KD,

  .output_max =  10.0,
  .output_min = -10.0,

  .stable_margin = 0,
};

static pid_config_S pid_conf_aim = {
  .kp = CONFIG_ENTRY_AIM_KP,
  .ki = CONFIG_ENTRY_AIM_KI,
  .kd = CONFIG_ENTRY_AIM_KD,

  .output_max =  1.0,
  .output_min = -1.0,

  .stable_margin = 0.1,
};

static pid_data_S pid;
static double control_target;
static bool debug;

void control_init(void) {
  control_reset = true;
  debug = false;
  pid_init(&pid);
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

    case CONTROL_STATE_BRAKE:
      if(motor_isStable(M1) && motor_isStable(M2)) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }
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
      double input = fmod(control_target / 180.0 * M_PI - compass_getHeading() + M_PI, 2 * M_PI) - M_PI;
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
      double speed = config_get(CONFIG_ENTRY_PUSH_SPEED);
      int32_t target = MOTOR_MM_TO_COUNT(control_target);

      if(target < 0) {
        speed = -speed;
      }

      bool finished = true;
      for(motor_E motor = M1; motor <= M2; motor++) {
        if(abs(target) > abs(motor_getCount(motor))) {
          motor_setSpeed(motor, speed);
          finished = false;
        } else {
          motor_stop(motor);
        }
      }

      if(finished) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    case CONTROL_STATE_TURN:
    {
      double speed = config_get(CONFIG_ENTRY_PUSH_SPEED);
      double wheel_dist = config_get(CONFIG_ENTRY_WHEEL_DIST) / 2;
      double target = MOTOR_MM_TO_COUNT(wheel_dist * control_target / 180.0 * M_PI);

      if(target < 0) {
        speed = -speed;
      }

      bool finished = true;
      for(motor_E motor = M1; motor <= M2; motor++) {
        if(abs(target) > abs(motor_getCount(motor))) {
          motor_setSpeed(motor, motor == M1 ? -speed : speed);
          finished = false;
        } else {
          motor_stop(motor);
        }
      }

      if(finished) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    case CONTROL_STATE_ARC:
    {
      double radius = config_get(CONFIG_ENTRY_ARC_RADIUS);
      double wheel_dist = config_get(CONFIG_ENTRY_WHEEL_DIST) / 2;
      double ratio = 1 - wheel_dist / radius;
      double speed = config_get(CONFIG_ENTRY_PUSH_SPEED);
      double target = MOTOR_MM_TO_COUNT(radius * fabs(control_target) / 180.0 * M_PI * 2);

      double ratios[MOTOR_COUNT];
      if(control_target < 0) {
        ratios[M1] = 1;
        ratios[M2] = ratio;
      } else {
        ratios[M1] = ratio;
        ratios[M2] = 1;
      }

      bool finished = true;
      for(motor_E motor = M1; motor <= M2; motor++) {
        if(abs(target * ratios[motor]) > abs(motor_getCount(motor))) {
          motor_setSpeed(motor, speed * ratios[motor]);
          finished = false;
        } else {
          motor_stop(motor);
        }
      }

      if(finished) {
        control_setState(CONTROL_STATE_NEUTRAL);
      }

      break;
    }

    default:
      break;
  }

  control_reset = false;

  if(debug) {
    printf("%ld %.4lf %.4lf\n",
      HAL_GetTick(),
      sensor_getMean(),
      sensor_getVariance()
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
      case CONTROL_STATE_ARC:
        motor_resetCount(M1);
        motor_resetCount(M2);
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
