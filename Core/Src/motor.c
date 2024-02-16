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
  MOTOR_MODE_COUNT,
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

  int32_t count_target;
  double speed_target;

  int32_t count;
  double speed;

  pid_data_S pid;
} motor_data_S;

static const motor_definition_S motors[MOTOR_NUM] = {
  [M1] = {
    .dir_port          = MtrDvr_Dir1_GPIO_Port,
    .dir_pin           = MtrDvr_Dir1_Pin,
    .pwm_reg           = &TIM4->CCR3,
    .pwm_timer         = &htim4,
    .pwm_timer_channel = TIM_CHANNEL_3,
    .enc_reg           = &TIM3->CNT,
    .enc_timer         = &htim3,
    .flip_dir          = true,
    .flip_enc          = true,
  },
  [M2] = {
    .dir_port          = MtrDvr_Dir2_GPIO_Port,
    .dir_pin           = MtrDvr_Dir2_Pin,
    .pwm_reg           = &TIM4->CCR1,
    .pwm_timer         = &htim4,
    .pwm_timer_channel = TIM_CHANNEL_1,
    .enc_reg           = &TIM1->CNT,
    .enc_timer         = &htim1,
    .flip_dir          = false,
    .flip_enc          = false,
  },
};

static pid_config_S motor_speed_pid_config = {
   .kp = CONFIG_ENTRY_SPEED_KP,
   .ki = CONFIG_ENTRY_SPEED_KI,
   .kd = CONFIG_ENTRY_SPEED_KD,

   .output_max =  1.0,
   .output_min = -1.0,
};

static pid_config_S motor_count_pid_config = {
   .kp = CONFIG_ENTRY_COUNT_KP,
   .ki = CONFIG_ENTRY_COUNT_KI,
   .kd = CONFIG_ENTRY_COUNT_KD,

   .output_max =  0.5,
   .output_min = -0.5,
};

static motor_data_S motor_datas[MOTOR_NUM];

static void motor_setPWM(motor_E motor_id, double pwm);

void motor_init(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_NUM; motor_id++) {
    const motor_definition_S *motor = &motors[motor_id];
    motor_data_S *data = &motor_datas[motor_id];

    pid_init(&data->pid);

    data->mode = MOTOR_MODE_STOP;

    HAL_TIM_PWM_Start(motor->pwm_timer, motor->pwm_timer_channel);
    HAL_TIM_Encoder_Start(motor->enc_timer, TIM_CHANNEL_ALL);
  }

  motor_setEnabled(true);
}

void motor_run(void) {
  for(motor_E motor_id = M1; motor_id < MOTOR_NUM; motor_id++) {
    const motor_definition_S *motor = &motors[motor_id];
    motor_data_S *data = &motor_datas[motor_id];

    // update counter and speed
    uint16_t count_raw = motor->flip_enc ? (*motor->enc_reg) : (*motor->enc_reg ^ 0xFFFF);

    int32_t count_diff = (int16_t) (count_raw - (uint16_t) data->count);
    data->count += count_diff;

    const double alpha = config_get(CONFIG_ENTRY_SPEED_ALPHA);
    data->speed = alpha * data->speed + (1 - alpha) * count_diff;

    // state machine
    motor_mode_E next_mode = data->mode;
    switch(data->mode) {
      case MOTOR_MODE_STOP:
        motor_setPWM(motor_id, 0.0);
        break;

      case MOTOR_MODE_COUNT:
      {
        double pwm_limit = config_get(CONFIG_ENTRY_COUNT_PWM_LIMIT);
        data->pid.config.output_max = pwm_limit;
        data->pid.config.output_min = -pwm_limit;

        double error = (double) (data->count_target - data->count);

        if(data->last_mode != data->mode) {
          pid_reset(&data->pid, error);
        }

        double u = pid_update(&data->pid, error);

        motor_setPWM(motor_id, u);
      }
      break;

      case MOTOR_MODE_SPEED:
      {
        double error = data->speed_target - data->speed;

        if(data->last_mode != data->mode) {
          pid_reset(&data->pid, error);
        }

        double u = pid_update(&data->pid, error);

        motor_setPWM(motor_id, u);
      }

      default:
        break;
    }

    data->last_mode = data->mode;
    data->mode = next_mode;
  }
}

void motor_move(motor_E motor_id, int32_t count) {
  motor_data_S *data = &motor_datas[motor_id];
  data->pid.config = motor_count_pid_config;
  data->count_target = data->count + count;
  data->mode = MOTOR_MODE_COUNT;
}

void motor_setSpeed(motor_E motor_id, double speed) {
  motor_data_S *data = &motor_datas[motor_id];
  data->pid.config = motor_speed_pid_config;
  data->speed_target = speed;
  data->mode = MOTOR_MODE_SPEED;
}

void motor_stop(motor_E motor_id) {
  motor_data_S *data = &motor_datas[motor_id];
  data->mode = MOTOR_MODE_STOP;
}

bool motor_getFault(void) {
  return !HAL_GPIO_ReadPin(MtrDvr_Fault_GPIO_Port, MtrDvr_Fault_Pin);
}

void motor_setEnabled(bool enabled) {
  HAL_GPIO_WritePin(MtrDvr_EN_GPIO_Port, MtrDvr_EN_Pin, enabled ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void motor_setPWM(motor_E motor_id, double pwm) {
  const motor_definition_S *motor = &motors[motor_id];
  const bool reversed = (pwm < 0) != motor->flip_dir;

  HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, reversed ? GPIO_PIN_SET : GPIO_PIN_RESET);

  // currently setup for the PWM to take a number from 0 - 1024
  *motor->pwm_reg = (uint32_t) (MIN(ABS(pwm), 1.0) * 1024);
}
