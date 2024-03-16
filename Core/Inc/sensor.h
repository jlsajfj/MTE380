#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_PD_COUNT 6

void sensor_init(void);
void sensor_run(void);

bool sensor_valid(void);
double sensor_getResult(void);
double sensor_getMean(void);
double sensor_getVariance(void);

void sensor_calibrate_white(void);
void sensor_calibrate_black(void);

double sensor_getVBatt(void);
double sensor_getValue(uint32_t index);

#endif
