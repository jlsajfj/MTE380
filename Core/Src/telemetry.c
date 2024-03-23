#include "telemetry.h"
#include "motor.h"
#include "state.h"
#include "sensor.h"
#include "usart.h"
#include "helper.h"
#include "config.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define SYNC_COUNT 50
#define STREAM_INTERVAL 5

const char SB_ACK    = 0x06;
const char SB_NACK   = 0x07;
const char SB_STREAM = 0x0E;
const char SB_CONFIG = 0x0F;

typedef struct __attribute__((packed)) {
  uint8_t start_byte;
  int8_t motor_speed[2];
  int8_t motor_speed_target[2];
  int32_t motor_count[2];
  uint8_t sm_state;
  uint8_t sensor_data[6];
  uint8_t batt_voltage;
  uint8_t compass_heading;
  uint32_t timestamp;
} tele_stream_S;

static tele_stream_S tele_stream;
static bool tele_enabled;
static bool tele_dump;

static uint16_t tele_ack_count;
static uint16_t tele_stream_count;

void tele_init(void) {
  memset(&tele_stream, 0, sizeof(tele_stream_S));

  tele_enabled = false;
  tele_ack_count = 0;
  tele_stream_count = 0;
  tele_dump = false;
}

void tele_run(void) {
  if(!tele_enabled) return;

  if(tele_ack_count > 0) {
    _write(2, &SB_ACK, 1);
    tele_ack_count--;
    return;
  }

  tele_stream_count = (tele_stream_count + 1) % STREAM_INTERVAL;
  if(tele_stream_count == 0) {
    tele_stream.start_byte = SB_STREAM;

    tele_stream.motor_speed[0] = SATURATE(motor_getSpeed(M1) / 16, -1, 1) * 0x7F;
    tele_stream.motor_speed[1] = SATURATE(motor_getSpeed(M2) / 16, -1, 1) * 0x7F;

    tele_stream.motor_speed_target[0] = SATURATE(motor_getSpeedTarget(M1) / 16, -1, 1) * 0x7F;
    tele_stream.motor_speed_target[1] = SATURATE(motor_getSpeedTarget(M2) / 16, -1, 1) * 0x7F;

    tele_stream.motor_count[0] = motor_getCount(M1);
    tele_stream.motor_count[1] = motor_getCount(M2);

    tele_stream.sm_state = sm_getState();

    for(uint32_t i = 0; i < SENSOR_PD_COUNT; i++) {
      tele_stream.sensor_data[i] = SATURATE(sensor_getValue(i), 0, 1) * 0x7F;
    }

    tele_stream.batt_voltage = SATURATE(sensor_getVBatt() / 9.0, 0, 1) * 0xFF;
    tele_stream.compass_heading = 0;
    tele_stream.timestamp = HAL_GetTick();

    _write(2, (char*) &tele_stream, sizeof(tele_stream));
  }

  if(tele_dump) {
    _write(2, &SB_CONFIG, 1);
    _write(2, (char*) config_getPtr(CONFIG_ENTRY_START), sizeof(double) * CONFIG_ENTRY_COUNT);
    tele_dump = false;
  }
}

void tele_dumpConfig(void) {
  tele_dump = true;
}

void tele_sync(void) {
  tele_ack_count = SYNC_COUNT;
}

void tele_respond(bool ack) {
  _write(2, ack ? &SB_ACK : &SB_NACK, 1);
}

void tele_setEnabled(bool enable) {
  tele_enabled = enable;
  if(enable) {
    tele_sync();
  }
}

bool tele_isEnabled(void) {
  return tele_enabled;
}
