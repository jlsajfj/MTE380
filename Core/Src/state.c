#include "state.h"
#include "control.h"
#include "config.h"
#include "compass.h"
#include "sensor.h"
#include "servo.h"

#include <stdint.h>

static sm_state_E sm_state = SM_STATE_INIT;
static uint32_t sm_state_time;

static bool cal_finished;

void sm_init(void) {
  sm_setState(SM_STATE_STANDBY);
}

void sm_run(void) {
  const control_state_E control_state = control_getState();

  if(control_state == CONTROL_STATE_FOLLOW) {
    control_setTarget(config_get(CONFIG_ENTRY_MOTOR_SPEED));
  }

  switch(sm_state) {
    case SM_STATE_UNHOOK:
      if(sm_state_time >= 500) {
        sm_setState(SM_STATE_GOTO_TARGET);
      }
      break;

    case SM_STATE_GOTO_TARGET:
    {
      double mean = sensor_getMean();
      if(mean > config_get(CONFIG_ENTRY_FINISH_M)) {
        sm_setState(SM_STATE_AIM);
      }
      break;
    }

    case SM_STATE_AIM:
      control_setTarget(config_get(CONFIG_ENTRY_AIM_TARGET));
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_TURN_BACK);
      }
      break;

    case SM_STATE_TURN_BACK:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_GOTO_HOME);
      }
      break;

    case SM_STATE_GOTO_HOME:
    {
      double mean = sensor_getMean();
      if(mean < config_get(CONFIG_ENTRY_START_M)) {
        sm_setState(SM_STATE_TURN_FORWARD);
      }
      break;
    }

    case SM_STATE_TURN_FORWARD:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_BACKUP);
      }
      break;

    case SM_STATE_BACKUP:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_STANDBY);
      }
      break;

    case SM_STATE_CALIBRATE:
      if(sm_state_time >= 1000) {
        if(compass_addCalPoint()) {
          cal_finished = true;
        }
        sm_setState(SM_STATE_CALIBRATE_MOVE);
      }
      break;

    case SM_STATE_CALIBRATE_MOVE:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(cal_finished ? SM_STATE_STANDBY : SM_STATE_CALIBRATE);
      }
      break;

    default:
      break;
  }

  sm_state_time++;
}

void sm_setState(sm_state_E state) {
  // state transition
  if(state != sm_state) {
    // state end action
    switch(sm_state) {
      case SM_STATE_AIM:
        servo_setPosition(S1, config_get(CONFIG_ENTRY_SERVO_UNLOCK));
        break;

      case SM_STATE_BACKUP:
        servo_setPosition(S2, config_get(CONFIG_ENTRY_SERVO_LOCK));
        break;

      case SM_STATE_CALIBRATE_MOVE:
        control_setState(CONTROL_STATE_BRAKE);
        if(state != SM_STATE_CALIBRATE) {
          compass_setState(COMPASS_STATE_IDLE);
        }
        break;

      default:
        break;
    }

    //printf("state %d -> %d\n", sm_state, state);
    sm_state = state;

    // state start action
    switch(sm_state) {
      case SM_STATE_STANDBY:
        control_setState(CONTROL_STATE_NEUTRAL);
        break;

      case SM_STATE_UNHOOK:
        servo_setPosition(S2, config_get(CONFIG_ENTRY_SERVO_UNLOCK));
        break;

      case SM_STATE_GOTO_TARGET:
      case SM_STATE_GOTO_HOME:
        control_setState(CONTROL_STATE_FOLLOW);
        break;

      case SM_STATE_AIM:
        control_setState(CONTROL_STATE_HEADING);
        break;

      case SM_STATE_TURN_BACK:
        control_setTarget(config_get(CONFIG_ENTRY_TURNBACK));
        control_setState(CONTROL_STATE_TURN);
        break;

      case SM_STATE_TURN_FORWARD:
        control_setTarget(-180.0);
        control_setState(CONTROL_STATE_TURN);
        break;

      case SM_STATE_BACKUP:
        control_setTarget(config_get(CONFIG_ENTRY_BACKUP));
        control_setState(CONTROL_STATE_MOVE);
        break;

      case SM_STATE_CALIBRATE:
        compass_setState(COMPASS_STATE_CALIBRATE);
        cal_finished = false;
        break;

      case SM_STATE_CALIBRATE_MOVE:
        control_setTarget(360.0 / 8);
        control_setState(CONTROL_STATE_TURN);
        break;

      default:
        break;
    }

    sm_state_time = 0;
  }
}

sm_state_E sm_getState(void) {
  return sm_state;
}
