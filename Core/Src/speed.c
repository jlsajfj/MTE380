#include "speed.h"
#include "sensor.h"
#include "config.h"
#include "motor.h"
#include "helper.h"
#include "flash.h"

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
  speed_type_E type;
} speed_point_S;

static speed_point_S speed_points[MAX_SPEED_POINTS];
static speed_state_E speed_state;
static uint16_t speed_num_points;

static void speed_add_point(speed_type_E);

void speed_init(void) {
  speed_state = SPEED_STATE_NORMAL;
  speed_num_points = 0;
  speed_load();
}

void speed_run(void) {
  double  error = fabs(sensor_getResult());
  double  rise  = config_get(CONFIG_ENTRY_SPEED_RISE);
  double  fall  = config_get(CONFIG_ENTRY_SPEED_FALL);

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
        speed_add_point(SPEED_TYPE_SLOW);
        speed_state = SPEED_STATE_RECORD_SLOW;
      }
      break;

    case SPEED_STATE_RECORD_SLOW:
      if(error < fall) {
        speed_add_point(SPEED_TYPE_FAST);
        speed_state = SPEED_STATE_RECORD_FAST;
      }
      break;
  }
}

void speed_startRecord(void) {
  speed_num_points = 0;
  speed_add_point(SPEED_TYPE_FAST);
  speed_state = SPEED_STATE_RECORD_START;
}

void speed_stopRecord(void) {
  speed_add_point(SPEED_TYPE_FINISH);
  speed_state = SPEED_STATE_NORMAL;
}

speed_type_E speed_fromCount(int32_t count) {
  double slipped_count = count * config_get(CONFIG_ENTRY_SPEED_SLIP);
  speed_type_E type = SPEED_TYPE_SLOW;

  for(uint32_t i = 0; i < speed_num_points; i++) {
    int32_t c = speed_points[i].count;
    speed_type_E t = SPEED_TYPE_SLOW;

    switch(speed_points[i].type) {
      case SPEED_TYPE_FAST:
        c += MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_FAST_OFFSET));
        t  = SPEED_TYPE_FAST;
        break;

      case SPEED_TYPE_SLOW:
        c += MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_SLOW_OFFSET));
        t  = SPEED_TYPE_SLOW;
        break;

      case SPEED_TYPE_FINISH:
        c += MOTOR_MM_TO_COUNT(config_get(CONFIG_ENTRY_FINISH_OFFSET));
        t  = SPEED_TYPE_FINISH;
        break;
    }

    if(slipped_count >= c || i == 0) {
      type = t;
    }
  }

  return type;
}

double speed_fromType(speed_type_E type) {
  switch(type) {
    case SPEED_TYPE_SLOW:
      return config_get(CONFIG_ENTRY_SPEED_SLOW);

    case SPEED_TYPE_FAST:
      return config_get(CONFIG_ENTRY_SPEED_FAST);

    case SPEED_TYPE_FINISH:
      return config_get(CONFIG_ENTRY_SPEED_FINISH);
  }

  return config_get(CONFIG_ENTRY_SPEED_SLOW);
}

void speed_save(void) {
  flash_write(FLASH_ADDR_SPEED_POINTS, &speed_num_points, sizeof(speed_num_points));
  flash_write(FLASH_ADDR_SPEED_POINTS + sizeof(speed_num_points), &speed_points, speed_num_points * sizeof(speed_points[0]));
  puts("speed data saved");
}

void speed_load(void) {
  flash_read(FLASH_ADDR_SPEED_POINTS, &speed_num_points, sizeof(speed_num_points));

  printf("loading %d points\n", speed_num_points);

  if(speed_num_points == 0xFFFF) {
    speed_num_points = 0;
  } else {
    flash_read(FLASH_ADDR_SPEED_POINTS + sizeof(speed_num_points), &speed_points, speed_num_points * sizeof(speed_points[0]));
  }

  for(uint32_t i = 0; i < speed_num_points; i++) {
    printf("%10lf %d\n", speed_points[i].count / MOTOR_COUNT_PER_MM, speed_points[i].type);
  }
}

void speed_reset(void) {
  speed_num_points = 0;
}

static void speed_add_point(speed_type_E type) {
  if(speed_num_points < MAX_SPEED_POINTS) {
    int32_t count = (motor_getCount(M1) + motor_getCount(M2)) / 2;
    printf("%10lf %d\n", count / MOTOR_COUNT_PER_MM, type);
    speed_points[speed_num_points].count = count;
    speed_points[speed_num_points].type = type;
    speed_num_points++;
  }
}
