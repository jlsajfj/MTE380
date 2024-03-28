#include "config.h"
#include "flash.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
  const char *name;
  double default_value;
} config_entry_S;

static config_entry_S config_entries[CONFIG_ENTRY_COUNT] = {
  [CONFIG_ENTRY_SENSOR_WHITE_0] = { "white_0",     4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_1] = { "white_1",     4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_2] = { "white_2",     4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_3] = { "white_3",     4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_4] = { "white_4",     4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_5] = { "white_5",     4000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_0] = { "black_0",     2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_1] = { "black_1",     2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_2] = { "black_2",     2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_3] = { "black_3",     2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_4] = { "black_4",     2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_5] = { "black_5",     2000.0  },
  [CONFIG_ENTRY_SENSOR_ALPHA]   = { "alpha",          0.2  },
  [CONFIG_ENTRY_SENSOR_GAIN_0]  = { "gain_0",         1.0  },
  [CONFIG_ENTRY_SENSOR_GAIN_1]  = { "gain_1",         3.0  },
  [CONFIG_ENTRY_SENSOR_GAIN_2]  = { "gain_2",         5.0  },
  [CONFIG_ENTRY_MOTOR_SPEED]    = { "speed",          0.1  },
  [CONFIG_ENTRY_CTRL_KP]        = { "kp",             1.0  },
  [CONFIG_ENTRY_CTRL_KI]        = { "ki",             0.0  },
  [CONFIG_ENTRY_CTRL_KD]        = { "kd",             0.0  },
  [CONFIG_ENTRY_SERVO_LOCK]     = { "servo_lock",     0.8  },
  [CONFIG_ENTRY_SERVO_UNLOCK]   = { "servo_unlock",   0.4  },
  [CONFIG_ENTRY_SPEED_KP]       = { "sp",             1.0  },
  [CONFIG_ENTRY_SPEED_KI]       = { "si",             0.0  },
  [CONFIG_ENTRY_SPEED_KD]       = { "sd",             0.0  },
  [CONFIG_ENTRY_SPEED_ALPHA]    = { "sa",             0.5  },
  [CONFIG_ENTRY_SPEED_SLIP]     = { "slip",           1.0  },
  [CONFIG_ENTRY_COUNT_KI]       = { "pi",             0.0  },
  [CONFIG_ENTRY_COUNT_KD]       = { "pd",             0.0  },
  [CONFIG_ENTRY_COUNT_SP]       = { "move_speed",     0.3  },
  [CONFIG_ENTRY_HOME_MEAN]      = { "home_mean",      1.0  },
  [CONFIG_ENTRY_HOME_VAR]       = { "home_var",       0.01 },
  [CONFIG_ENTRY_TARGET_MEAN]    = { "target_mean",    0.6  },
  [CONFIG_ENTRY_TARGET_VAR]     = { "target_var",     0.06 },
  [CONFIG_ENTRY_RECORD_MEAN]    = { "record_mean",    1.0  },
  [CONFIG_ENTRY_RECORD_VAR]     = { "record_var",     0.00 },
  [CONFIG_ENTRY_RECORD_SPEED]   = { "record_speed",   3.0  },
  [CONFIG_ENTRY_COMPASS_V1]     = { "compass_cal_v1", 0.0  },
  [CONFIG_ENTRY_COMPASS_W0]     = { "compass_cal_w0", 1.0  },
  [CONFIG_ENTRY_COMPASS_W1]     = { "compass_cal_w1", 0.0  },
  [CONFIG_ENTRY_COMPASS_W2]     = { "compass_cal_w2", 1.0  },
  [CONFIG_ENTRY_COMPASS_W3]     = { "compass_cal_w3", 0.0  },
  [CONFIG_ENTRY_AIM_KP]         = { "ap",             1.0  },
  [CONFIG_ENTRY_AIM_KI]         = { "ai",             0.0  },
  [CONFIG_ENTRY_AIM_KD]         = { "ad",             0.0  },
  [CONFIG_ENTRY_WHEEL_DIST]     = { "wheel_dist",   123.0  },
  [CONFIG_ENTRY_TURNBACK]       = { "turnback",    -135.0  },
  [CONFIG_ENTRY_BACKIN]         = { "backin",       -50.0  },
  [CONFIG_ENTRY_BACKUP]         = { "backup",      -100.0  },
  [CONFIG_ENTRY_PUSH_1]         = { "push1",        100.0  },
  [CONFIG_ENTRY_PUSH_2]         = { "push2",         50.0  },
  [CONFIG_ENTRY_PUSH_SPEED]     = { "push_speed",    50.0  },
  [CONFIG_ENTRY_AIM_TARGET]     = { "target",       -45.0  },
  [CONFIG_ENTRY_ARC_RADIUS]     = { "arc_radius",   100.0  },
  [CONFIG_ENTRY_SPEED_RISE]     = { "speed_rise",     1.0  },
  [CONFIG_ENTRY_SPEED_FALL]     = { "speed_fall",     0.5  },
  [CONFIG_ENTRY_FAST_OFFSET]    = { "fast_offset",  -80.0  },
  [CONFIG_ENTRY_SLOW_OFFSET]    = { "slow_offset",  -80.0  },
  [CONFIG_ENTRY_FINISH_OFFSET]  = { "finish_offset", -80.0 },
  [CONFIG_ENTRY_SPEED_FAST]     = { "speed_fast",      2.0 },
  [CONFIG_ENTRY_SPEED_SLOW]     = { "speed_slow",      2.0 },
  [CONFIG_ENTRY_SPEED_FINISH]   = { "speed_finish",    0.5 },
};

static double config_values[CONFIG_ENTRY_COUNT];

static config_id_E config_getIdByName(char *name);

void config_set(config_id_E id, double value) {
  if(id >= 0 && id < CONFIG_ENTRY_COUNT) {
    config_values[id] = value;
  }
}

double config_get(config_id_E id) {
  if(id >= 0 && id < CONFIG_ENTRY_COUNT) {
    return config_values[id];
  }
  return 0.0;
}

double *config_getPtr(config_id_E id) {
  if(id >= 0 && id < CONFIG_ENTRY_COUNT) {
    return config_values + id;
  }
  return NULL;
}

double config_getByName(char *name) {
  return config_get(config_getIdByName(name));
}

void config_setByName(char *name, double value) {
  config_set(config_getIdByName(name), value);
}

void config_load(void) {
  flash_read(FLASH_ADDR_CONFIG, &config_values, sizeof(config_values));

  for(uint16_t i = 0; i < CONFIG_ENTRY_COUNT; i++) {
    if(isnan(config_values[i])) {
      printf("loading default for %s\n", config_entries[i].name);
      config_values[i] = config_entries[i].default_value;
    }
    printf("%16s = %lf\n", config_entries[i].name, config_values[i]);
  }
}

void config_save(void) {
  flash_write(FLASH_ADDR_CONFIG, &config_values, sizeof(config_values));
}

void config_print(void) {
  for(uint16_t i = 0; i < CONFIG_ENTRY_COUNT; i++) {
    printf("%16s = %lf\n", config_entries[i].name, config_values[i]);
  }
}

static config_id_E config_getIdByName(char *name) {
  for(uint16_t i = 0; i < CONFIG_ENTRY_COUNT; i++) {
    if(strcmp(config_entries[i].name, name) == 0) {
      return i;
    }
  }
  return CONFIG_ENTRY_COUNT;
}
