#include "motor.h"
#include "main.h"
#include "tim.h"
#include "pid.h"
#include "config.h"
#include "helper.h"

#include "stm32f4xx_hal.h"

typedef enum {
  MOTOR_MODE_STOP,
  MOTOR_MODE_SPEED,
  MOTOR_MODE_PWM,
} motor_mode_E;

typedef struct {
  GPIO_TypeDef *dir_port;
  uint16_t dir_pin;

  __IO uint32_t *pwm_reg;
  TIM_HandleTypeDef *pwm_timer;
  uint32_t pwm_timer_channel;

  __IO uint32_t *enc_reg;
  TIM_HandleTypeDef *enc_timer;

  bool flip_dir;
  bool flip_enc;
} motor_definition_S;

typedef struct {
  motor_mode_E mode;
  motor_mode_E last_mode;

  double speed_target;

  int32_t count;
  double speed;

  double last_pwm;
  pid_data_S pid;
} motor_data_S;

static const motor_definition_S motors[MOTOR_COUNT] = {
  [M1] = {
    .dir_port          = MtrDvr_Dir1_GPIO_Port,
    .dir_pin           = MtrDvr_Dir1_Pin,
    .pwm_reg           = &TIM4->CCR2,
    .pwm_timer         = &htim4,
    .pwm_timer_channel = TIM_CHANNEL_2,
    .enc_reg           = &TIM1->CNT,
    .enc_timer         = &htim1,
    .flip_dir          = true,
    .flip_enc          = false,
  },
  [M2] = {
    .dir_port          = MtrDvr_Dir2_GPIO_Port,
    .dir_pin           = MtrDvr_Dir2_Pin,
    .pwm_reg           = &TIM4->CCR1,
    .pwm_timer         = &htim4,
    .pwm_timer_channel = TIM_CHANNEL_1,
    .enc_reg           = &TIM3->CNT,
    .enc_timer         = &htim3,
    .flip_dir          = false,
    .flip_enc          = true,
  },
};

static pid_config_S motor_pid_config = {
   .kp = CONFIG_ENTRY_SPEED_KP,
   .ki = CONFIG_ENTRY_SPEED_KI,
   .kd = CONFIG_ENTRY_SPEED_KD,

   .output_max =  1.0,
   .output_min = -1.0,
};

static motor_data_S motor_datas[MOTOR_COUNT];

static void motor_setPWM_private(motor_E motor_id, double pwm);

void motor_init(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_COUNT; motor_id++) {
    const motor_definition_S *motor = &motors[motor_id];
    motor_data_S *data = &motor_datas[motor_id];

    data->pid.config = motor_pid_config;
    pid_init(&data->pid);

    data->mode = MOTOR_MODE_STOP;

    HAL_TIM_PWM_Start(motor->pwm_timer, motor->pwm_timer_channel);
    HAL_TIM_Encoder_Start(motor->enc_timer, TIM_CHANNEL_ALL);
  }
}

void motor_run(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_COUNT; motor_id++) {
    const motor_definition_S *motor = &motors[motor_id];
    motor_data_S *data = &motor_datas[motor_id];

    // update counter and speed
    uint16_t count_raw = motor->flip_enc ? (*motor->enc_reg) : (-*motor->enc_reg);

    int32_t count_diff = (int16_t) (count_raw - (uint16_t) data->count);
    data->count += count_diff;

    const double alpha = config_get(CONFIG_ENTRY_SPEED_ALPHA);
    data->speed = alpha * data->speed + (1 - alpha) * count_diff;

    // state machine
    switch(data->mode) {
      case MOTOR_MODE_STOP:
        if(data->last_mode != data->mode) {
          motor_setPWM(motor_id, 0.0);
        }
        break;

      case MOTOR_MODE_SPEED:
      {
        double error = data->speed_target - data->speed;

        bool reset = data->last_mode != data->mode;
        double pwm = pid_update(&data->pid, error, reset);

        motor_setPWM_private(motor_id, pwm);

        break;
      }

      default:
        break;
    }

    data->last_mode = data->mode;
  }
}

void motor_setSpeed(motor_E motor_id, double speed) {
  motor_data_S *data = &motor_datas[motor_id];
  data->speed_target = speed;
  data->mode = MOTOR_MODE_SPEED;
}

void motor_stop(motor_E motor_id) {
  motor_data_S *data = &motor_datas[motor_id];
  data->mode = MOTOR_MODE_STOP;
}

int32_t motor_getCount(motor_E motor_id) {
  return motor_datas[motor_id].count;
}

double motor_getSpeed(motor_E motor_id) {
  return motor_datas[motor_id].speed;
}

void motor_setPWM(motor_E motor_id, double pwm) {
  motor_data_S *data = &motor_datas[motor_id];
  data->mode = MOTOR_MODE_PWM;
  motor_setPWM_private(motor_id, pwm);
}

static void motor_setPWM_private(motor_E motor_id, double pwm) {
  const motor_definition_S *motor = &motors[motor_id];
  motor_data_S *data = &motor_datas[motor_id];
  const bool reversed = (pwm < 0) != motor->flip_dir;

  HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, reversed ? GPIO_PIN_SET : GPIO_PIN_RESET);

  data->last_pwm = pwm;
  *motor->pwm_reg = (uint32_t) (MIN(ABS(pwm), 1.0) * (motor->pwm_timer->Init.Period + 1));
}
