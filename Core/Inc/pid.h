#ifndef __PID_H__
#define __PID_H__

#include "config.h"

#include <stdbool.h>

typedef struct {
   config_id_E kp;
   config_id_E ki;
   config_id_E kd;

   double output_max;
   double output_min;
} pid_config_S;

typedef struct {
   pid_config_S config;
   double error_accu;
   double error_last;
   double output;
   bool stable;
} pid_data_S;

void pid_init(pid_data_S *data);
double pid_update(pid_data_S *data, double error, bool reset);

#endif
