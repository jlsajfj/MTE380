#ifndef __CONFIG_H__
#define __CONFIG_H__

// double only because we only need it for now and I'm lazy

typedef enum {
   CONFIG_ENTRY_RESERVED,
   CONFIG_ENTRY_SENSOR_ALPHA,
   CONFIG_ENTRY_SENSOR_GAIN_0,
   CONFIG_ENTRY_SENSOR_GAIN_1,
   CONFIG_ENTRY_SENSOR_GAIN_2,
   CONFIG_ENTRY_SENSOR_WHITE_0,
   CONFIG_ENTRY_SENSOR_WHITE_1,
   CONFIG_ENTRY_SENSOR_WHITE_2,
   CONFIG_ENTRY_SENSOR_WHITE_3,
   CONFIG_ENTRY_SENSOR_WHITE_4,
   CONFIG_ENTRY_SENSOR_WHITE_5,
   CONFIG_ENTRY_SENSOR_BLACK_0,
   CONFIG_ENTRY_SENSOR_BLACK_1,
   CONFIG_ENTRY_SENSOR_BLACK_2,
   CONFIG_ENTRY_SENSOR_BLACK_3,
   CONFIG_ENTRY_SENSOR_BLACK_4,
   CONFIG_ENTRY_SENSOR_BLACK_5,
   CONFIG_ENTRY_MOTOR_SPEED,
   CONFIG_ENTRY_CTRL_KP,
   CONFIG_ENTRY_CTRL_KI,
   CONFIG_ENTRY_CTRL_KD,
   CONFIG_ENTRY_SERVO_LOCK,
   CONFIG_ENTRY_SERVO_UNLOCK,
   CONFIG_ENTRY_COUNT,
} config_id_E;

void config_set(config_id_E id, double value);
double config_get(config_id_E id);

double config_getByName(char *name);
void config_setByName(char *name, double value);

void config_load(void);
void config_save(void);
void config_print(void);

#endif