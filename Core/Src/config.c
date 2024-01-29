#include "config.h"
#include "flash.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
        config_id_E id;
        const char *name;
        double default_value;
} config_entry_S;

static config_entry_S config_entries[CONFIG_ENTRY_COUNT] = {
  [CONFIG_ENTRY_CTRL_KP]        = { CONFIG_ENTRY_CTRL_KP,         "kp",         1.0  },
  [CONFIG_ENTRY_SENSOR_ALPHA]   = { CONFIG_ENTRY_SENSOR_ALPHA,    "alpha",      0.8  },
  [CONFIG_ENTRY_SENSOR_GAIN_0]  = { CONFIG_ENTRY_SENSOR_GAIN_0,   "gain_0",    -3.5  },
  [CONFIG_ENTRY_SENSOR_GAIN_1]  = { CONFIG_ENTRY_SENSOR_GAIN_1,   "gain_1",    -1.0  },
  [CONFIG_ENTRY_SENSOR_GAIN_2]  = { CONFIG_ENTRY_SENSOR_GAIN_2,   "gain_2",    -0.5  },
  [CONFIG_ENTRY_SENSOR_GAIN_3]  = { CONFIG_ENTRY_SENSOR_GAIN_3,   "gain_3",     0.5  },
  [CONFIG_ENTRY_SENSOR_GAIN_4]  = { CONFIG_ENTRY_SENSOR_GAIN_4,   "gain_4",     1.0  },
  [CONFIG_ENTRY_SENSOR_GAIN_5]  = { CONFIG_ENTRY_SENSOR_GAIN_5,   "gain_5",     3.5  },
  [CONFIG_ENTRY_SENSOR_WHITE_0] = { CONFIG_ENTRY_SENSOR_WHITE_0,  "white_0", 4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_1] = { CONFIG_ENTRY_SENSOR_WHITE_1,  "white_1", 4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_2] = { CONFIG_ENTRY_SENSOR_WHITE_2,  "white_2", 4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_3] = { CONFIG_ENTRY_SENSOR_WHITE_3,  "white_3", 4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_4] = { CONFIG_ENTRY_SENSOR_WHITE_4,  "white_4", 4000.0  },
  [CONFIG_ENTRY_SENSOR_WHITE_5] = { CONFIG_ENTRY_SENSOR_WHITE_5,  "white_5", 4000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_0] = { CONFIG_ENTRY_SENSOR_BLACK_0,  "black_0", 2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_1] = { CONFIG_ENTRY_SENSOR_BLACK_1,  "black_1", 2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_2] = { CONFIG_ENTRY_SENSOR_BLACK_2,  "black_2", 2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_3] = { CONFIG_ENTRY_SENSOR_BLACK_3,  "black_3", 2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_4] = { CONFIG_ENTRY_SENSOR_BLACK_4,  "black_4", 2000.0  },
  [CONFIG_ENTRY_SENSOR_BLACK_5] = { CONFIG_ENTRY_SENSOR_BLACK_5,  "black_5", 2000.0  },
};

static union {
  double values[CONFIG_ENTRY_COUNT];
  uint8_t raw[sizeof(double) * CONFIG_ENTRY_COUNT];
} config_values;

static config_id_E config_getIdByName(char *name);

void config_set(config_id_E id, double value) {
  if(id >= 0 && id < CONFIG_ENTRY_COUNT) {
    config_values.values[id] = value;
  }
}

double config_get(config_id_E id) {
  if(id >= 0 && id < CONFIG_ENTRY_COUNT) {
    return config_values.values[id];
  }
  return NAN;
}

double config_getByName(char *name) {
  return config_get(config_getIdByName(name));
}

void config_setByName(char *name, double value) {
  config_set(config_getIdByName(name), value);
}

void config_load(void) {
  flash_read(FLASH_ADDR_CONFIG, config_values.raw, sizeof(config_values));

  for(uint16_t i = 0; i < CONFIG_ENTRY_COUNT; i++) {
    if(isnan(config_values.values[i])) {
      printf("loading default for %s\n", config_entries[i].name);
      config_values.values[i] = config_entries[i].default_value;
    }
    printf("%16s = %lf\n", config_entries[i].name, config_values.values[i]);
  }
}

void config_save(void) {
  flash_write(FLASH_ADDR_CONFIG, config_values.raw, sizeof(config_values));
}

static config_id_E config_getIdByName(char *name) {
  for(uint16_t i = 0; i < CONFIG_ENTRY_COUNT; i++) {
    if(strcmp(config_entries[i].name, name) == 0) {
      return i;
    }
  }
  return CONFIG_ENTRY_COUNT;
}
