#ifndef __CONFIG_H__
#define __CONFIG_H__

// double only because we only need it for now and I'm lazy

typedef enum {
	CONFIG_ENTRY_SENSOR_ALPHA,
	CONFIG_ENTRY_CTRL_KP,
	CONFIG_ENTRY_COUNT,
} config_id_E;

void config_set(config_id_E id, double value);
double config_get(config_id_E id);

double config_getByName(char *name);
void config_setByName(char *name, double value);

void config_load(void);
void config_save(void);

#endif
