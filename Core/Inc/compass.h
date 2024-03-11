#ifndef __COMPASS_H__
#define __COMPASS_H__

#include <stdbool.h>

typedef enum {
  COMPASS_STATE_INIT,
  COMPASS_STATE_IDLE,
  COMPASS_STATE_RUN,
  COMPASS_STATE_CALIBRATE,
} compass_state_E;

void compass_init(void);
void compass_run(void);

void compass_setState(compass_state_E state);
bool compass_addCalPoint(void);

double compass_getHeading(void);

#endif
