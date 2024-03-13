#include "state.h"
#include "control.h"
#include "config.h"
#include "compass.h"

#include <stdint.h>

static sm_state_E sm_state = SM_STATE_INIT;
static uint32_t sm_state_counter;

void sm_init(void) {
  sm_setState(SM_STATE_STANDBY);
}

void sm_run(void) {
  const control_state_E control_state = control_getState();

  switch(sm_state) {
    case SM_STATE_STANDBY:
      if(control_state == CONTROL_STATE_FOLLOW) {
        control_setTarget(config_get(CONFIG_ENTRY_MOTOR_SPEED));
      }
      break;

    case SM_STATE_CALIBRATE:
      if(sm_state_counter >= 1000) {
        if(compass_addCalPoint()) {
          sm_setState(SM_STATE_STANDBY);
        } else {
          sm_setState(SM_STATE_CALIBRATE_MOVE);
        }
      }
      break;

    case SM_STATE_CALIBRATE_MOVE:
      if(control_state == CONTROL_STATE_NEUTRAL) {
        sm_setState(SM_STATE_CALIBRATE);
      }
      break;

    default:
      break;
  }

  sm_state_counter++;
}

void sm_setState(sm_state_E state) {
  // state transition
  if(state != sm_state) {
    // state end action
    switch(sm_state) {
      case SM_STATE_CALIBRATE_MOVE:
        control_setState(CONTROL_STATE_BRAKE);
        break;

      case SM_STATE_CALIBRATE:
        if(state != SM_STATE_CALIBRATE_MOVE) {
          control_setState(CONTROL_STATE_TURN);
          compass_setState(COMPASS_STATE_IDLE);
        }
        break;

      default:
        break;
    }

    sm_state = state;

    // state start action
    switch(sm_state) {
      case SM_STATE_CALIBRATE:
        compass_setState(COMPASS_STATE_CALIBRATE);
        break;

      case SM_STATE_CALIBRATE_MOVE:
        control_setTarget(360.0 / 8);
        control_setState(CONTROL_STATE_TURN);
        break;

      default:
        break;
    }

    sm_state_counter = 0;
  }
}

sm_state_E sm_getState(void) {
  return sm_state;
}
