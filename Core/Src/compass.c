#include "compass.h"
#include "i2c.h"
#include "helper.h"
#include "config.h"

#include "easyMatrix.h"

#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define COMPASS_ADDR 0x3C
#define CAL_POINTS 8

static compass_state_E compass_state = COMPASS_STATE_INIT;

static uint8_t i2c_buff[6];
static bool buff_ready;

static double x = 0.0, y = 0.0;
static double compass_heading;
static double alpha = 0;

static uint8_t cal_count;
static double cal_matrix_data[CAL_POINTS*5];

static void compass_read(void);
static void compass_computeCal(void);

void compass_init(void) {
  uint8_t config[] = {
    0x78, // 8 sample average, 75Hz data rate, no bias
    0x20, // 0.92 Mg/LSB
    0x00, // continuous measurement mode
  };

  if(HAL_I2C_Mem_Write(&hi2c1, COMPASS_ADDR, 0x0, I2C_MEMADD_SIZE_8BIT, config, sizeof(config), 100) != HAL_OK) {
    puts("compass init failed");
    return;
  }

  buff_ready = false;
  HAL_I2C_Mem_Read_IT(&hi2c1, COMPASS_ADDR, 0x3, I2C_MEMADD_SIZE_8BIT, i2c_buff, sizeof(i2c_buff));

  compass_setState(COMPASS_STATE_IDLE);
}

void compass_run(void) {
  switch(compass_state) {
    case COMPASS_STATE_RUN:
    {
      compass_read();

      double B_data[2] = { x, y };
      struct easyMatrix B = {
        .rows = 2,
        .cols = 1,
        .element = B_data,
      };

      double Bcal_data[2];
      struct easyMatrix Bcal = {
        .rows = 2,
        .cols = 1,
        .element = Bcal_data,
      };

      struct easyMatrix V = {
        .rows = 2,
        .cols = 1,
        .element = config_getPtr(CONFIG_ENTRY_COMPASS_V0),
      };

      struct easyMatrix W = {
        .rows = 2,
        .cols = 2,
        .element = config_getPtr(CONFIG_ENTRY_COMPASS_W0),
      };

      subMatrix(&B, &V, &B);
      multiMatrix(&W, &B, &Bcal);

      compass_heading = atan2(Bcal_data[1], Bcal_data[0]);

      //printf("%10.4lf %10.4lf %10.4lf %10.4lf %10.4lf\n", x, y, Bcal_data[0], Bcal_data[1], compass_heading);

      break;
    }

    case COMPASS_STATE_CALIBRATE:
      compass_read();
      break;

    default:
      break;
  }
}

void compass_setState(compass_state_E state) {
  if(state != compass_state) {
    switch(compass_state) {
      case COMPASS_STATE_CALIBRATE:
        alpha = 0.0;
        compass_computeCal();
        break;

      default:
        break;
    }

    compass_state = state;

    switch(compass_state) {
      case COMPASS_STATE_CALIBRATE:
        puts("starting calibration");
        alpha = 0.8;
        cal_count = 0;
        break;

      default:
        break;
    }
  }
}

bool compass_addCalPoint(void) {
  if(cal_count < CAL_POINTS) {
    cal_matrix_data[5*cal_count+0] = x*x;
    cal_matrix_data[5*cal_count+1] = x*y;
    cal_matrix_data[5*cal_count+2] = y*y;
    cal_matrix_data[5*cal_count+3] = x;
    cal_matrix_data[5*cal_count+4] = y;
    cal_count++;
  }
  return cal_count >= CAL_POINTS;
}

double compass_getHeading(void) {
  return compass_heading;
}

static void compass_read(void) {
  if(!buff_ready) return;

  x = alpha * x + (1 - alpha) * (int16_t) ((i2c_buff[0] << 8) | i2c_buff[1]);
  y = alpha * y + (1 - alpha) * (int16_t) ((i2c_buff[4] << 8) | i2c_buff[5]);

  buff_ready = false;
  HAL_I2C_Mem_Read_IT(&hi2c1, COMPASS_ADDR, 0x3, I2C_MEMADD_SIZE_8BIT, i2c_buff, sizeof(i2c_buff));
}

static void compass_computeCal(void) {
  // https://www.nxp.com/docs/en/application-note/AN4246.pdf
  // https://www.ensta-bretagne.fr/lebars/Share/RI_magnetometer.pdf

  // C
  struct easyMatrix C = {
    .rows = cal_count,
    .cols = 5,
    .element = cal_matrix_data,
  };

  // C^T
  double Ct_data[5*CAL_POINTS];
  struct easyMatrix Ct = {
    .rows = 5,
    .cols = cal_count,
    .element = Ct_data,
  };

  transMatrix(&C, &Ct);

  // P = C^T * C
  double P_data[5*5];
  struct easyMatrix P = {
    .rows = 5,
    .cols = 5,
    .element = P_data,
  };

  multiMatrix(&Ct, &C, &P);

  // b = C^T * [-1] x cal_count
  double b_data[5];
  struct easyMatrix b = {
    .rows = 5,
    .cols = 1,
    .element = b_data,
  };

  for(int i = 0; i < b.rows; i++) {
    b_data[i] = 0;
    for(int j = 0; j < Ct.cols; j++) {
      b_data[i] += -1 * Ct_data[i*Ct.cols+j];
    }
  }

  // P = LU
  double L_data[5*5];
  struct easyMatrix L = {
    .rows = 5,
    .cols = 5,
    .element = L_data,
  };

  double U_data[5*5];
  struct easyMatrix U = {
    .rows = 5,
    .cols = 5,
    .element = U_data,
  };

  getLUMatrix(&P, &L, &U);

  // Ly = b, Ux = y
  double x_data[5];
  struct easyMatrix x = {
    .rows = 5,
    .cols = 1,
    .element = x_data,
  };

  solveLMatrix(&L, &b, &x);
  solveUMatrix(&U, &x, &x);

  // M = [x[0], x[1]; x[1], x[2]]
  double *M_data = P_data; // stack reuse
  struct easyMatrix M = {
    .rows = 2,
    .cols = 2,
    .element = M_data,
  };

  M_data[0] = x_data[0];
  M_data[1] = x_data[1];
  M_data[2] = x_data[1];
  M_data[3] = x_data[2];

  // V = -1/2 M^(-1) [x[3]; x[4]]
  double *V_data = config_getPtr(CONFIG_ENTRY_COMPASS_V0);
  struct easyMatrix V = {
    .rows = 2,
    .cols = 1,
    .element = V_data,
  };

  double p = -2 * (M_data[0] * M_data[3] - M_data[1] * M_data[2]);
  V_data[0] = ( x_data[3] * M_data[3] - x_data[4] * M_data[1]) / p;
  V_data[1] = (-x_data[3] * M_data[2] + x_data[4] * M_data[0]) / p;

  // M' = 1 / (V^T M V - 1) * M
  double q =
    V_data[0] * V_data[0] * M_data[0] +
    V_data[0] * V_data[1] * (M_data[1] + M_data[2]) +
    V_data[1] * V_data[1] * M_data[3] - 1;
  scaleMatrix(1 / q, &M, &M);

  // M' = LU = LDL^T
  L.rows = L.cols = 2;
  U.rows = U.cols = 2;
  getLUMatrix(&M, &L, &U);

  if(U_data[0] < 0 || U_data[3] < 0) {
    puts("calibration failed");
    return;
  }

  // L^T = D^(-1/2) U
  double *D_data = b_data; // stack reuse
  struct easyMatrix D = {
    .rows = 2,
    .cols = 2,
    .element = b_data,
  };
  D_data[0] = 1 / sqrt(U_data[0]);
  D_data[1] = 0;
  D_data[2] = 0;
  D_data[3] = 1 / sqrt(U_data[3]);

  // W^-1 = L^T = D^{-1/2}U
  double *W_data = config_getPtr(CONFIG_ENTRY_COMPASS_W0);
  struct easyMatrix W = {
    .rows = 2,
    .cols = 2,
    .element = W_data,
  };

  multiMatrix(&D, &U, &W);

  puts("C");
  dumpMatrix(&C);
  puts("V");
  dumpMatrix(&V);
  puts("W");
  dumpMatrix(&W);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  buff_ready = true;
}
