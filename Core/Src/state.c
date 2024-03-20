#include "state.h"
#include "control.h"
#include "config.h"
#include "compass.h"
#include "sensor.h"
#include "servo.h"
#include "motor.h"
#include "helper.h"
#include "speed.h"

#include <stdint.h>

static sm_state_E sm_state = SM_STATE_INIT;
static uint32_t sm_state_time;

static bool cal_finished;

typedef struct {
  int32_t count;
  double speed;
} speed_point_S;

void sm_init(void) {
  sm_setState(SM_STATE_STANDBY);
}

void sm_run(void) {
  const control_state_E control_state = control_getState();

  switch(sm_state) {
    case SM_STATE_UNHOOK:
      if(sm_state_time >= 100) {
        sm_setState(SM_STATE_GOTO_TARGET);
      }
      break;

    case SM_STATE_GOTO_TARGET:
    {
      double mean = sensor_getMean();
      double var  = sensor_getVariance();
      double mean_th = config_get(CONFIG_ENTRY_TARGET_MEAN);
      double var_th  = config_get(CONFIG_ENTRY_TARGET_VAR);

      if(mean < mean_th && var < var_th) {
        sm_setState(SM_STATE_TARGET_BRAKE);
      }

      int32_t count = (motor_getCount(M1) + motor_getCount(M2)) / 2;
      double speed = speed_fromCount(count);
      control_setTarget(speed * config_get(CONFIG_ENTRY_MOTOR_SPEED));

      break;
    }

    case SM_STATE_TARGET_BRAKE:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_AIM);
      }
      break;

    case SM_STATE_PUSH_1:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_AIM);
      }
      break;

    case SM_STATE_AIM:
      if(control_state == CONTROL_STATE_NEUTRAL || sm_state_time > 5000) {
        sm_setState(SM_STATE_KICK);
      }
      break;

    case SM_STATE_PUSH_2:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_KICK);
      }
      break;

    case SM_STATE_KICK:
      if(sm_state_time > 100) {
        servo_setPosition(S1, config_get(CONFIG_ENTRY_SERVO_UNLOCK));
      }
      if(sm_state_time > 200) {
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
      double var  = sensor_getVariance();
      double mean_th = config_get(CONFIG_ENTRY_HOME_MEAN);
      double var_th  = config_get(CONFIG_ENTRY_HOME_VAR);
      if(mean < mean_th && var < var_th) {
        sm_setState(SM_STATE_HOME_BRAKE);
      }

      control_setTarget(config_get(CONFIG_ENTRY_MOTOR_SPEED));

      break;
    }

    case SM_STATE_HOME_BRAKE:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_BACKUP);
      }
      break;

    case SM_STATE_BACKUP:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_TURN_FORWARD);
      }
      break;

    case SM_STATE_TURN_FORWARD:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_BACKIN);
      }
      break;

    case SM_STATE_BACKIN:
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

    case SM_STATE_RECORD:
    {
      double mean = sensor_getMean();
      double var  = sensor_getVariance();
      double mean_th = config_get(CONFIG_ENTRY_TARGET_MEAN);
      double var_th  = config_get(CONFIG_ENTRY_TARGET_VAR);

      if(mean < mean_th && var < var_th) {
        sm_setState(SM_STATE_STANDBY);
      }

      control_setTarget(config_get(CONFIG_ENTRY_MOTOR_SPEED));

      break;
    }

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
      case SM_STATE_BACKIN:
        servo_setPosition(S2, config_get(CONFIG_ENTRY_SERVO_LOCK));
        break;

      case SM_STATE_CALIBRATE_MOVE:
        control_setState(CONTROL_STATE_BRAKE);
        if(state != SM_STATE_CALIBRATE) {
          compass_setState(COMPASS_STATE_IDLE);
        }
        break;

      case SM_STATE_RECORD:
        speed_stopRecord();
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
        motor_resetCount(M1);
        motor_resetCount(M2);
        control_setState(CONTROL_STATE_FOLLOW);
        break;

      case SM_STATE_TARGET_BRAKE:
      case SM_STATE_HOME_BRAKE:
        control_setState(CONTROL_STATE_BRAKE);
        break;

      case SM_STATE_AIM:
        //control_setState(CONTROL_STATE_HEADING);
        control_setTarget(config_get(CONFIG_ENTRY_AIM_TARGET));
        control_setState(CONTROL_STATE_ARC);
        break;

      case SM_STATE_PUSH_1:
        control_setTarget(config_get(CONFIG_ENTRY_PUSH_1));
        control_setState(CONTROL_STATE_MOVE);
        break;

      case SM_STATE_PUSH_2:
        control_setTarget(config_get(CONFIG_ENTRY_PUSH_2));
        control_setState(CONTROL_STATE_MOVE);
        break;

      case SM_STATE_KICK:
        //servo_setPosition(S1, config_get(CONFIG_ENTRY_SERVO_UNLOCK));
        control_setTarget(config_get(CONFIG_ENTRY_PUSH_2));
        control_setState(CONTROL_STATE_MOVE);
        break;

      case SM_STATE_TURN_BACK:
        control_setTarget(config_get(CONFIG_ENTRY_TURNBACK));
        control_setState(CONTROL_STATE_TURN);
        break;

      case SM_STATE_BACKUP:
        control_setTarget(config_get(CONFIG_ENTRY_BACKUP));
        control_setState(CONTROL_STATE_MOVE);
        break;

      case SM_STATE_TURN_FORWARD:
        control_setTarget(-180.0);
        control_setState(CONTROL_STATE_TURN);
        break;

      case SM_STATE_BACKIN:
        control_setTarget(config_get(CONFIG_ENTRY_BACKIN));
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

      case SM_STATE_RECORD:
        motor_resetCount(M1);
        motor_resetCount(M2);
        control_setState(CONTROL_STATE_FOLLOW);
        speed_startRecord();
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
