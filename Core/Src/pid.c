#include "pid.h"
#include "config.h"

#include <stdbool.h>

void pid_init(pid_data_S *data) {
   data->error_accu = 0;
   data->error_last = 0;
}

double pid_update(pid_data_S *data, double error) {
   double kp = config_get(data->kp_config);
   double ki = config_get(data->ki_config);
   double kd = config_get(data->kd_config);

   bool clamp = (
     (data->output_last > data->output_max ||
      data->output_last < data->output_min) &&
      data->output_last * error < 0
   );

   double accu = 0;
   if(!clamp) {
      accu = data->error_accu + error;
      data->error_accu = accu;
   }

   double diff = error - data->error_last;
   data->error_last = error;

   double output = kp * error + ki * accu + kd * diff;
   data->output_last = output;

   return output;
}
