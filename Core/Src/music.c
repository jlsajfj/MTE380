#include "music.h"
#include "motor.h"

// tones generated by https://github.com/LenShustek/miditones
#define PROGMEM static
#include "icecream.c"

typedef enum {
  MUSIC_STATE_STOP,
  MUSIC_STATE_PLAY,
} music_state_E;

music_state_E music_state;
uint32_t music_position;
uint32_t music_delay;

static double music_toFreq(uint8_t note);

void music_init(void) {
  music_state = MUSIC_STATE_STOP;
  music_position = 0;
  music_delay = 0;
}

void music_run(void) {
  switch(music_state) {
    case MUSIC_STATE_PLAY:
    {
      if(music_delay > 0) {
        music_delay--;
        break;
      }

      uint8_t cmd = score[music_position++];

      if(cmd == 0xF0) { // end of score
        music_stop();

      } else if(cmd == 0xE0) { // end of score, restart
        music_position = 0;

      } else if((cmd & 0xF0) == 0x90) { // play note
        uint8_t generator = cmd & 0x0F;
        if(generator < MOTOR_COUNT) {
          motor_buzz(generator, music_toFreq(score[music_position++]));
        }

      } else if((cmd & 0xF0) == 0x80) { // stop note
        uint8_t generator = cmd & 0x0F;
        if(generator < MOTOR_COUNT) {
          motor_stop(generator);
        }

      } else if((cmd & 0xF0) == 0x00) { // delay
        music_delay = (cmd & 0x0F) << 8;
        music_delay |= score[music_position++];
      }

      if(music_position >= sizeof(score)) {
        music_stop();
      }

      break;
    }

    default:
      break;
  }
}

void music_play(void) {
  music_state = MUSIC_STATE_PLAY;
}

void music_pause(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_COUNT; motor_id++) {
    motor_stop(motor_id);
  }
  music_state = MUSIC_STATE_STOP;
}

void music_stop(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_COUNT; motor_id++) {
    motor_stop(motor_id);
  }
  music_state = MUSIC_STATE_STOP;
  music_position = 0;
  music_delay = 0;
}

static double music_toFreq(uint8_t note) {
   return pow(2, (note - 69) / 12.0) * 440 * 2; // raise by an octave, otherwise motor driver overheats
}
