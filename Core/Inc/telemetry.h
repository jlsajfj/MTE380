#ifndef __TELEMETRY_H__
#define __TELEMETRY_H__

#include <stdbool.h>

void tele_init(void);
void tele_run(void);

void tele_sync(void);
void tele_respond(bool ack);
void tele_dumpConfig(void);

void tele_setEnabled(bool);
bool tele_isEnabled(void);

#endif
