#include "config.h"
#include "flash.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
	config_id_E id;
	const char *name;
} config_entry_S;

static config_entry_S config_entries[CONFIG_ENTRY_COUNT] = {
  [CONFIG_ENTRY_SENSOR_ALPHA] = { CONFIG_ENTRY_SENSOR_ALPHA, "alpha" },
  [CONFIG_ENTRY_CTRL_KP]      = { CONFIG_ENTRY_CTRL_KP, "kp" },
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
