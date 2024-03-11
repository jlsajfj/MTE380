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

static control_state_E control_state;
static control_state_E control_state_next;

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
static uint32_t state_counter;
static double control_target;
static bool debug;

void control_init(void) {
  control_state = CONTROL_STATE_INIT;
  control_state_next = CONTROL_STATE_STOP;
  debug = false;
  pid_init(&pid);
}

void control_run(void) {
  bool reset_pid = false;

  // state transition
  if(control_state_next != control_state) {
    // state end action
    switch(control_state) {
      case CONTROL_STATE_CALIBRATE_MOVE:
        motor_setSpeed(M1, 0);
        motor_setSpeed(M2, 0);
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
        compass_setState(COMPASS_STATE_RUN);
        motor_stop(M1);
        motor_stop(M2);
        break;

      case CONTROL_STATE_CALIBRATE:
        compass_setState(COMPASS_STATE_CALIBRATE);
        break;

      case CONTROL_STATE_CALIBRATE_MOVE:
        motor_setSpeed(M1,  1);
        motor_setSpeed(M2, -1);
        break;

      case CONTROL_STATE_AIM:
        compass_setState(COMPASS_STATE_RUN);
        pid.config = pid_conf_aim;
        reset_pid = true;
        break;

      default:
        break;
    }

    state_counter = 0;
  }

  switch(control_state) {
    case CONTROL_STATE_STOP:
      break;

    case CONTROL_STATE_RUN:
    {
      double mean = sensor_getMean();
      double var = sensor_getVariance();

      double start_mean = config_get(CONFIG_ENTRY_START_M);
      double start_var = config_get(CONFIG_ENTRY_START_V);
      double finish_mean = config_get(CONFIG_ENTRY_FINISH_M);
      double finish_var = config_get(CONFIG_ENTRY_FINISH_V);

      double mean_thres = config_get(CONFIG_ENTRY_MEAN_THRESHOLD);
      double var_thres = config_get(CONFIG_ENTRY_VAR_THRESHOLD);

      if((fabs(mean - start_mean)  < mean_thres && fabs(var - start_var)  < var_thres) ||
         (fabs(mean - finish_mean) < mean_thres && fabs(var - finish_var) < var_thres)
      ) {
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
      if(state_counter >= 1000) {
        //control_state_next = CONTROL_STATE_STOP;
        //break;
        puts(".");
        if(compass_addCalPoint()) {
          control_state_next = CONTROL_STATE_STOP;
        } else {
          control_state_next = CONTROL_STATE_CALIBRATE_MOVE;
        }
      }
      break;

    case CONTROL_STATE_CALIBRATE_MOVE:
      if(state_counter >= 300) {
        control_state_next = CONTROL_STATE_CALIBRATE;
      }
      break;

    case CONTROL_STATE_AIM:
    {
      double input = fmod(control_target - compass_getHeading() + M_PI, 2 * M_PI) - M_PI;
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

    default:
      break;
  }

  state_counter++;

  if(debug) {
    printf("%ld %.4lf %.4f %.4lf %.4lf %.4lf\n",
      state_counter,
      sensor_getResult(),
      sensor_getMean(),
      sensor_getVariance(),
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
  control_target = heading;
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
