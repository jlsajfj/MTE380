#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdbool.h>

void sensor_init(void);
void sensor_run(void);

bool sensor_valid(void);
double sensor_getResult(void);

void sensor_calibrate_white(void);
void sensor_calibrate_black(void);
void sensor_calibrate_load(void);
void sensor_calibrate_save(void);

#endif
