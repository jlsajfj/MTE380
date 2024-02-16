#include "pid.h"
#include "config.h"
#include "helper.h"

#include <stdbool.h>

void pid_init(pid_data_S *data) {
   data->error_accu = 0;
   data->error_last = 0;
}

double pid_update(pid_data_S *data, double error) {
   double kp = config_get(data->config.kp);
   double ki = config_get(data->config.ki);
   double kd = config_get(data->config.kd);

   double diff = error - data->error_last;
   data->error_last = error;

   double pd_output = kp * error + kd * diff;

   if(ki != 0) {
      double accu_min = (data->config.output_min - pd_output) / ki;
      double accu_max = (data->config.output_max - pd_output) / ki;
      double accu = SATURATE(data->error_accu + error, accu_min, accu_max);
      data->error_accu = accu;
      data->output = pd_output + ki * accu;
   } else {
      data->output = SATURATE(pd_output, data->config.output_min, data->config.output_max);
   }

   return data->output;
}

void pid_reset(pid_data_S *data, double error) {
   double kp = config_get(data->config.kp);
   double ki = config_get(data->config.ki);

   // exclude derivative as the value would be way off between pid transitions
   data->error_last = error;
   data->error_accu = (data->output - kp * error) / ki;
}
