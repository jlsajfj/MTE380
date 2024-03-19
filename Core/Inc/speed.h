#ifndef __SPEED_H__
#define __SPEED_H__

#include <stdint.h>
#include <stdbool.h>

void speed_init(void);
void speed_run(void);

void speed_startRecord(void);
void speed_stopRecord(void);
double speed_fromCount(int32_t step);

void speed_save(void);
void speed_load(void);

#endif
