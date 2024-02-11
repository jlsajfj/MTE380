#ifndef __PID_H__
#define __PID_H__

#include "config.h"

typedef struct {
   config_id_E kp_config;
   config_id_E ki_config;
   config_id_E kd_config;

   double output_max;
   double output_min;

   double error_accu;
   double error_last;
   double output_last;
} pid_data_S;

void pid_init(pid_data_S *data);
double pid_update(pid_data_S *data, double error);

#endif
