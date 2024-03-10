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

static control_state_E control_state = CONTROL_STATE_STOP;
static control_state_E control_state_next = CONTROL_STATE_STOP;

static uint32_t state_start;

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

  .output_max =  3.0,
  .output_min = -3.0,
};

static pid_data_S pid;
static double aim_target;
static bool debug;

void control_init(void) {
  debug = false;
  pid_init(&pid);
}

void control_run(void) {
  bool reset_pid = false;

  // state transition
  if(control_state_next != control_state) {
    // state end action
    switch(control_state) {
      case CONTROL_STATE_CALIBRATE:
      case CONTROL_STATE_AIM:
        compass_setState(COMPASS_STATE_IDLE);
        break;

      default:
        break;
    }

    control_state = control_state_next;

    // state start action
    switch(control_state) {
      case CONTROL_STATE_RUN:
        pid.config = pid_conf_sensor;
        reset_pid = true;

      case CONTROL_STATE_STOP:
        motor_stop(M1);
        motor_stop(M2);
        break;

      case CONTROL_STATE_CALIBRATE:
        compass_setState(COMPASS_STATE_CALIBRATE);
        motor_setSpeed(M1, 0.5);
        motor_setSpeed(M2, -0.5);
        break;

      case CONTROL_STATE_AIM:
        compass_setState(COMPASS_STATE_RUN);
        pid.config = pid_conf_aim;
        reset_pid = true;
        break;

      default:
        break;
    }

    state_start = HAL_GetTick();
  }

  switch(control_state) {
    case CONTROL_STATE_STOP:
      break;

    case CONTROL_STATE_RUN:
    {
      double average = sensor_getAverage();
      double start_thres = config_get(CONFIG_ENTRY_START);
      double finish_thres = config_get(CONFIG_ENTRY_FINISH);
      if(average < start_thres || average > finish_thres) {
        control_setState(CONTROL_STATE_STOP);
        break;
      }

      double input = sensor_getResult();
      double u = pid_update(&pid, input, reset_pid);

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

    case CONTROL_STATE_CALIBRATE:
      if(HAL_GetTick() - state_start > 5000) {
        control_state_next = CONTROL_STATE_STOP;
      }
      break;

    case CONTROL_STATE_AIM:
    {
      double input = fmod(aim_target - compass_getHeading() + M_PI, 2 * M_PI) - M_PI;
      double u = pid_update(&pid, input, reset_pid);
      double speed = config_get(CONFIG_ENTRY_AIM_SP);

      if(u > 0) {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      } else {
        motor_setSpeed(M1, speed - u / 2);
        motor_setSpeed(M2, speed + u / 2);
      }

      break;
    }
  }

  if(debug) {
    printf("%ld %.4lf %.4lf %ld %ld %.4lf %.4lf %.4lf %.4lf\n",
      HAL_GetTick(),
      sensor_getResult(),
      sensor_getAverage(),
      motor_getCount(M1),
      motor_getCount(M2),
      motor_getSpeed(M1),
      motor_getSpeed(M2),
      compass_getHeading(),
      sensor_getVBatt()
    );
  }
}

void control_setState(control_state_E state) {
  control_state_next = state;
}

control_state_E control_getState(void) {
  return control_state;
}

void control_aim(double heading) {
  aim_target = heading;
  control_setState(CONTROL_STATE_AIM);
}

void control_debug(int8_t d) {
  if(d == 0)
    debug = false;
  else if(d == 1)
    debug = true;
  else if(d == -1)
    debug ^= 1;
}
