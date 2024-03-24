#ifndef __CONFIG_H__
#define __CONFIG_H__

// double only because we only need it for now and I'm lazy

typedef enum {
   CONFIG_ENTRY_START = 0,
   CONFIG_ENTRY_SENSOR_WHITE_0 = 0,
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
   CONFIG_ENTRY_SENSOR_ALPHA,
   CONFIG_ENTRY_SENSOR_GAIN_0,
   CONFIG_ENTRY_SENSOR_GAIN_1,
   CONFIG_ENTRY_SENSOR_GAIN_2,
   CONFIG_ENTRY_MOTOR_SPEED,
   CONFIG_ENTRY_CTRL_KP,
   CONFIG_ENTRY_CTRL_KI,
   CONFIG_ENTRY_CTRL_KD,
   CONFIG_ENTRY_SERVO_LOCK,
   CONFIG_ENTRY_SERVO_UNLOCK,
   CONFIG_ENTRY_SPEED_KP,
   CONFIG_ENTRY_SPEED_KI,
   CONFIG_ENTRY_SPEED_KD,
   CONFIG_ENTRY_SPEED_ALPHA,
   CONFIG_ENTRY_SPEED_SLIP,
   CONFIG_ENTRY_COUNT_KI,
   CONFIG_ENTRY_COUNT_KD,
   CONFIG_ENTRY_COUNT_SP,
   CONFIG_ENTRY_HOME_MEAN,
   CONFIG_ENTRY_HOME_VAR,
   CONFIG_ENTRY_TARGET_MEAN,
   CONFIG_ENTRY_TARGET_VAR,
   CONFIG_ENTRY_RECORD_MEAN,
   CONFIG_ENTRY_RECORD_VAR,
   CONFIG_ENTRY_COMPASS_V0,
   CONFIG_ENTRY_COMPASS_V1,
   CONFIG_ENTRY_COMPASS_W0,
   CONFIG_ENTRY_COMPASS_W1,
   CONFIG_ENTRY_COMPASS_W2,
   CONFIG_ENTRY_COMPASS_W3,
   CONFIG_ENTRY_AIM_KP,
   CONFIG_ENTRY_AIM_KI,
   CONFIG_ENTRY_AIM_KD,
   CONFIG_ENTRY_WHEEL_DIST,
   CONFIG_ENTRY_TURNBACK,
   CONFIG_ENTRY_BACKIN,
   CONFIG_ENTRY_BACKUP,
   CONFIG_ENTRY_PUSH_1,
   CONFIG_ENTRY_PUSH_2,
   CONFIG_ENTRY_PUSH_SPEED,
   CONFIG_ENTRY_AIM_TARGET,
   CONFIG_ENTRY_ARC_RADIUS,
   CONFIG_ENTRY_SPEED_RISE,
   CONFIG_ENTRY_SPEED_FALL,
   CONFIG_ENTRY_FAST_OFFSET,
   CONFIG_ENTRY_SLOW_OFFSET,
   CONFIG_ENTRY_FINISH_OFFSET,
   CONFIG_ENTRY_SPEED_FAST,
   CONFIG_ENTRY_SPEED_SLOW,
   CONFIG_ENTRY_SPEED_FINISH,

   CONFIG_ENTRY_COUNT,
   CONFIG_ENTRY_NONE,
} config_id_E;

void config_set(config_id_E id, double value);
double config_get(config_id_E id);
double *config_getPtr(config_id_E id);

double config_getByName(char *name);
void config_setByName(char *name, double value);

void config_load(void);
void config_save(void);
void config_print(void);

#endif
