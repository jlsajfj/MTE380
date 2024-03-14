#ifndef __STATE_H__
#define __STATE_H__

typedef enum {
  SM_STATE_INIT,
  SM_STATE_STANDBY,

  SM_STATE_UNHOOK,
  SM_STATE_GOTO_TARGET,
  SM_STATE_TARGET_BRAKE,
  SM_STATE_AIM,
  SM_STATE_PUSH,
  SM_STATE_TURN_BACK,
  SM_STATE_GOTO_HOME,
  SM_STATE_HOME_BRAKE,
  SM_STATE_TURN_FORWARD,
  SM_STATE_BACKUP,

  SM_STATE_CALIBRATE,
  SM_STATE_CALIBRATE_MOVE,

  SM_STATE_COUNT,
} sm_state_E;

void sm_init(void);
void sm_run(void);

void sm_setState(sm_state_E state);
sm_state_E sm_getState(void);

#endif