#ifndef __SPEED_H__
#define __SPEED_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  SPEED_TYPE_FAST,
  SPEED_TYPE_SLOW,
  SPEED_TYPE_FINISH,
} speed_type_E;

void speed_init(void);
void speed_run(void);

void speed_startRecord(void);
void speed_stopRecord(void);
speed_type_E speed_fromCount(int32_t step);
double speed_fromType(speed_type_E type);

void speed_save(void);
void speed_load(void);

#endif
