#include "pid.h"
#include "config.h"
#include "helper.h"

#include <stdbool.h>
#include <math.h>

void pid_init(pid_data_S *data) {
   data->error_accu = 0;
   data->error_last = 0;
}

double pid_update(pid_data_S *data, double error, bool reset) {
   double kp = config_get(data->config.kp);
   double ki = config_get(data->config.ki);
   double kd = config_get(data->config.kd);

   double diff;
   if(reset) {
      diff = 0.0;
      data->error_last = error;
      data->error_accu = 0.0;
      data->stable = false;
   } else {
      diff = error - data->error_last;
      data->stable = fabs(diff) < data->config.stable_margin && fabs(error) < data->config.stable_margin;
   }

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

   if(isnan(data->output)) {
      data->output = 0.0;
   }

   return data->output;
}
