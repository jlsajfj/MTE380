#ifndef __SPEED_H__
#define __SPEED_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  SPEED_TYPE_FAST,
  SPEED_TYPE_SLOW,
  SPEED_TYPE_FINISH,
} speed_type_E;

typedef struct {
  int32_t count;
  speed_type_E type;
} speed_point_S;

void speed_init(void);
void speed_run(void);

void speed_startRecord(void);
void speed_stopRecord(void);
speed_type_E speed_fromCount(int32_t step);
double speed_fromType(speed_type_E type);
const speed_point_S *speed_getPoints(uint16_t *count);

void speed_save(void);
void speed_load(void);
void speed_reset(void);

#endif
