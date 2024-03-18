#include "speed.h"
#include "sensor.h"
#include "config.h"
#include "motor.h"
#include "helper.h"

#include <stdio.h>

#define MAX_SPEED_POINTS 64

typedef enum {
  SPEED_STATE_NORMAL,
  SPEED_STATE_RECORD_START,
  SPEED_STATE_RECORD_SLOW,
  SPEED_STATE_RECORD_FAST,
} speed_state_E;


typedef struct {
  int32_t count;
  double speed;
} speed_point_S;

static speed_point_S speed_points[MAX_SPEED_POINTS];
static speed_state_E speed_state;
static uint16_t speed_num_points;

static void speed_add_point(int32_t count, double speed);

void speed_init(void) {
  speed_state = SPEED_STATE_NORMAL;
  speed_num_points = 0;
}

void speed_run(void) {
  double  error = fabs(sensor_getResult());
  double  rise  = config_get(CONFIG_ENTRY_SPEED_RISE);
  double  fall  = config_get(CONFIG_ENTRY_SPEED_FALL);
  int32_t count = (motor_getCount(M1) + motor_getCount(M2)) / 2;

  switch(speed_state) {
    case SPEED_STATE_NORMAL:
      break;

    case SPEED_STATE_RECORD_START:
      if(error < fall) {
        speed_state = SPEED_STATE_RECORD_FAST;
      }
      break;

    case SPEED_STATE_RECORD_FAST:
      if(error > rise) {
        int32_t offset = MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_SLOW_OFFSET));
        double speed = config_get(CONFIG_ENTRY_SPEED_SLOW);
        speed_add_point(count + offset, speed);
        speed_state = SPEED_STATE_RECORD_SLOW;
      }
      break;

    case SPEED_STATE_RECORD_SLOW:
      if(error < fall) {
        int32_t offset = MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_FAST_OFFSET));
        double speed = config_get(CONFIG_ENTRY_SPEED_FAST);
        speed_add_point(count + offset, speed);
        speed_state = SPEED_STATE_RECORD_FAST;
      }
      break;
  }
}

void speed_startRecord(void) {
  int32_t count = (motor_getCount(M1) + motor_getCount(M2)) / 2;
  double speed = config_get(CONFIG_ENTRY_SPEED_FAST);
  speed_num_points = 0;
  speed_add_point(count, speed);
  speed_state = SPEED_STATE_RECORD_START;
}

void speed_stopRecord(void) {
  int32_t offset_slow = MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_SLOW_OFFSET));
  int32_t offset_finish = MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_FINISH_OFFSET));
  int32_t count = (motor_getCount(M1) + motor_getCount(M2)) / 2;
  double speed_slow   = config_get(CONFIG_ENTRY_SPEED_SLOW);
  double speed_finish = config_get(CONFIG_ENTRY_SPEED_FINISH);
  speed_add_point(count + offset_slow + offset_finish, speed_slow);
  speed_add_point(count + offset_finish, speed_finish);
  speed_state = SPEED_STATE_NORMAL;

  puts("result");
  for(int32_t i = 0; i < speed_num_points; i++) {
    printf("%10lf %10.2lf\n", speed_points[i].count / MOTOR_COUNT_PER_MM, speed_points[i].speed);
  }
}

double speed_fromCount(int32_t count, bool reverse) {
  (void) reverse;

  for(int i = 0; i < speed_num_points; i++) {
    if(count < speed_points[i].count) {
      return speed_points[MAX(i-1, 0)].speed;
    }
  }

  return speed_points[MAX(speed_num_points-1, 0)].speed;
}

void speed_save(void) {
}

void speed_load(void) {
}

static void speed_add_point(int32_t count, double speed) {
  if(speed_num_points < MAX_SPEED_POINTS) {
    while(speed_num_points > 0 && count < speed_points[speed_num_points-1].count) {
      speed_num_points--;
    }

    speed_points[speed_num_points].count = count;
    speed_points[speed_num_points].speed = speed;
    speed_num_points++;
  }
}
