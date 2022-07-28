/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: extendedKalmanFilter1RC.c
 *
 * MATLAB Coder version            : 5.1
 * C/C++ source code generated on  : 28-Jul-2022 14:01:04
 */

/* Include Files */
#include "extendedKalmanFilter.h"
#include <math.h>
#include <string.h>

/* Function Definitions */
/*
 * U             = curent at instant t
 *  X             = state vector [SOC, V1, V2]
 *  Parameters    = parameters of the equivalent circuit of the battery
 *  SOCOCV        = 11th order polynomial for the SOC-OCV curve
 *  dSOCOCV       = derivative of SOC-OCV curve for matrix C
 *  P_x / P_z     = matrices d’estimation de la covariance de l’erreur
 *  Q_x / Q_z     = matrices de covariance
 *  R_x / R_z     = bruits gaussiens de la mesure
 *  DeltaT        = période d'échantillonage
 *  Qn_rated      = capacité nominale des batteries
 * Arguments    : double b_I
 *                double I2
 *                double X[2]
 *                double Z[3]
 *                const double SOCOCV[5]
 *                const double dSOCOCV[4]
 *                double Vt_Actual
 *                double P_x[4]
 *                double P_z[9]
 *                const double Q_x[4]
 *                const double Q_z[9]
 *                double *betha_x
 *                double *betha_z
 *                double *alpha_x
 *                double *alpha_z
 *                double rho_x
 *                double rho_z
 *                double DeltaT
 *                double Qn_rated
 * Return Type  : double
 */
double extendedKalmanFilter(double b_I, double I2, double X[2], double Z[3],
  const double SOCOCV[5], const double dSOCOCV[4], double Vt_Actual, double P_x
  [4], double P_z[9], const double Q_x[4], const double Q_z[9], double *betha_x,
  double *betha_z, double *alpha_x, double *alpha_z, double rho_x, double rho_z,
  double DeltaT, double Qn_rated)
{
  double P_zb[9];
  double d_I[9];
  double dv[6];
  double Fk[4];
  double b_Fk[4];
  double C_z[3];
  double KalmanGain_z[3];
  double KalmanGain_x[2];
  double Error_x;
  double OCV;
  double SOC;
  double TerminalVoltage;
  double Xb_idx_0;
  double Xb_idx_1;
  double Zb_idx_0;
  double Zb_idx_1;
  double Zb_idx_2;
  double b_C_z;
  double betha_xb;
  double betha_zb;
  double d;
  double d1;
  double dOCV;
  int I_tmp;
  int b_i;
  int i;
  signed char c_I[9];

  /*  States */
  SOC = X[0];

  /*  Parameters */
  /*  Coef in A : Fk : State transition matrix */
  betha_xb = exp(-DeltaT / Z[2]);

  /*  Coef in B : Gk : Input control matrix */
  /*  Coef in mat : dXk/dThetak */
  /*  Linearizing the model by linearizing the C matrix */
  dOCV = SOC * (SOC * (SOC * dSOCOCV[0] + dSOCOCV[1]) + dSOCOCV[2]) + dSOCOCV[3];

  /*  dUoc(SOC)/dSOC at SOC = SOCk */
  if (X[0] > 1.0) {
    dOCV = 0.0;
  }

  /*  Hx : Jacobian measurement matrix of x */
  dv[1] = 0.0;
  d = DeltaT / (Z[2] * Z[2]);
  dv[3] = -b_I * (exp(d) - 1.0);
  dv[5] = d * (X[1] - Z[1] * b_I) * exp(DeltaT / Z[2]);
  for (i = 0; i < 3; i++) {
    I_tmp = i << 1;
    dv[I_tmp] = 0.0;
    KalmanGain_z[i] = dOCV * 0.0 + -dv[I_tmp + 1];
  }

  C_z[0] = -b_I + KalmanGain_z[0];
  C_z[1] = KalmanGain_z[1];
  C_z[2] = KalmanGain_z[2];

  /*  Htheta : Jacobian measurement matrix of theta */
  /*  Knowing SOC, we can compute OCV thanks to the relationship SOC-OCV */
  OCV = SOC * (SOC * (SOC * (SOC * SOCOCV[0] + SOCOCV[1]) + SOCOCV[2]) + SOCOCV
               [3]) + SOCOCV[4];

  /*  calculate the values of OCV at the given SOC, using the polynomial SOCOCV */
  if (X[0] > 1.0) {
    OCV = (((SOCOCV[0] + SOCOCV[1]) + SOCOCV[2]) + SOCOCV[3]) + SOCOCV[4];
  }

  /*  --- EKF ALGORITHM --- */
  /*  --> SOC ESTIMATION */
  /*  Fk : State transition matrix */
  Fk[0] = 1.0;
  Fk[2] = 0.0;
  Fk[1] = 0.0;
  Fk[3] = betha_xb;

  /*  Gk : Input control matrix */
  /* %% ---- PREDICTION ---- %%% */
  KalmanGain_x[0] = X[0] + 0.0 * X[1];
  KalmanGain_x[1] = 0.0 * X[0] + betha_xb * X[1];
  X[0] = -(DeltaT / Qn_rated) * I2;
  X[1] = Z[1] * (1.0 - betha_xb) * I2;

  /*  U_k-1 */
  for (i = 0; i < 2; i++) {
    X[i] += KalmanGain_x[i];
    d = Fk[i + 2];
    I_tmp = (int)Fk[i];
    b_Fk[i] = (double)I_tmp * P_x[0] + d * P_x[1];
    b_Fk[i + 2] = (double)I_tmp * P_x[2] + d * P_x[3];
  }

  for (i = 0; i < 2; i++) {
    d = b_Fk[i + 2];
    d1 = b_Fk[i];
    P_x[i] = (d1 + d * 0.0) + Q_x[i];
    P_x[i + 2] = (d1 * 0.0 + d * betha_xb) + Q_x[i + 2];
  }

  for (i = 0; i < 9; i++) {
    P_z[i] += Q_z[i];
  }

  *alpha_x *= rho_x;
  *betha_x *= rho_x;
  *alpha_z *= rho_z;
  *betha_z *= rho_z;

  /* %% ---- UPDATE ---- %%% */
  *alpha_x += 0.5;
  betha_xb = *betha_x;
  *alpha_z += 0.5;
  betha_zb = *betha_z;

  /*  Terminal voltage estimation */
  TerminalVoltage = (OCV - Z[0] * b_I) - X[1];

  /*  Calculate the Vt error */
  Error_x = Vt_Actual - TerminalVoltage;
  d = -P_x[3];
  for (b_i = 0; b_i < 5; b_i++) {
    /*  Measurement variances */
    /*  State estimate and its covariance */
    SOC = 1.0 / (((dOCV * P_x[0] + -P_x[1]) * dOCV + -(dOCV * P_x[2] + d)) +
                 betha_xb / *alpha_x);
    d1 = (P_x[0] * dOCV + -P_x[2]) * SOC;
    KalmanGain_x[0] = d1;
    Xb_idx_0 = X[0] + d1 * Error_x;
    d1 = (P_x[1] * dOCV + d) * SOC;
    Xb_idx_1 = X[1] + d1 * Error_x;
    b_Fk[0] = 1.0 - KalmanGain_x[0] * dOCV;
    b_Fk[1] = 0.0 - d1 * dOCV;
    b_Fk[2] = 0.0 - (-KalmanGain_x[0]);
    b_Fk[3] = 1.0 - (-d1);
    for (i = 0; i < 2; i++) {
      d1 = b_Fk[i + 2];
      b_C_z = b_Fk[i];
      Fk[i] = b_C_z * P_x[0] + d1 * P_x[1];
      Fk[i + 2] = b_C_z * P_x[2] + d1 * P_x[3];
    }

    /*  Battery parameters estimate and its covariance */
    b_C_z = 0.0;
    for (i = 0; i < 3; i++) {
      KalmanGain_z[i] = (P_z[i] * C_z[0] + P_z[i + 3] * C_z[1]) + P_z[i + 6] *
        C_z[2];
      b_C_z += ((C_z[0] * P_z[3 * i] + C_z[1] * P_z[3 * i + 1]) + C_z[2] * P_z[3
                * i + 2]) * C_z[i];
    }

    SOC = 1.0 / (b_C_z + betha_zb / *alpha_z);
    d1 = KalmanGain_z[0] * SOC;
    KalmanGain_z[0] = d1;
    Zb_idx_0 = Z[0] + d1 * Error_x;
    d1 = KalmanGain_z[1] * SOC;
    KalmanGain_z[1] = d1;
    Zb_idx_1 = Z[1] + d1 * Error_x;
    d1 = KalmanGain_z[2] * SOC;
    Zb_idx_2 = Z[2] + d1 * Error_x;
    for (i = 0; i < 9; i++) {
      c_I[i] = 0;
    }

    c_I[0] = 1;
    c_I[4] = 1;
    c_I[8] = 1;
    b_C_z = KalmanGain_z[0];
    SOC = KalmanGain_z[1];
    for (i = 0; i < 3; i++) {
      betha_xb = C_z[i];
      d_I[3 * i] = (double)c_I[3 * i] - b_C_z * betha_xb;
      I_tmp = 3 * i + 1;
      d_I[I_tmp] = (double)c_I[I_tmp] - SOC * betha_xb;
      I_tmp = 3 * i + 2;
      d_I[I_tmp] = (double)c_I[I_tmp] - d1 * betha_xb;
    }

    for (i = 0; i < 3; i++) {
      d1 = d_I[i];
      b_C_z = d_I[i + 3];
      SOC = d_I[i + 6];
      for (I_tmp = 0; I_tmp < 3; I_tmp++) {
        P_zb[i + 3 * I_tmp] = (d1 * P_z[3 * I_tmp] + b_C_z * P_z[3 * I_tmp + 1])
          + SOC * P_z[3 * I_tmp + 2];
      }
    }

    /*  Parameters for the measurement noise variances estimation */
    /*  Terminal voltage estimation */
    /*  Calculate the Vt error */
    SOC = Vt_Actual - ((OCV - Zb_idx_0 * b_I) - Xb_idx_1);
    SOC = 0.5 * (SOC * SOC);
    betha_xb = (*betha_x + SOC) + 0.5 * ((dOCV * Fk[0] + -Fk[1]) * dOCV + -(dOCV
      * Fk[2] + -Fk[3]));
    b_C_z = 0.0;
    for (i = 0; i < 3; i++) {
      b_C_z += ((C_z[0] * P_zb[3 * i] + C_z[1] * P_zb[3 * i + 1]) + C_z[2] *
                P_zb[3 * i + 2]) * C_z[i];
    }

    betha_zb = (*betha_z + SOC) + 0.5 * b_C_z;
  }

  *betha_x = betha_xb;
  *betha_z = betha_zb;
  X[0] = Xb_idx_0;
  X[1] = Xb_idx_1;
  P_x[0] = Fk[0];
  P_x[1] = Fk[1];
  P_x[2] = Fk[2];
  P_x[3] = Fk[3];
  Z[0] = Zb_idx_0;
  Z[1] = Zb_idx_1;
  Z[2] = Zb_idx_2;
  memcpy(&P_z[0], &P_zb[0], 9U * sizeof(double));
  return TerminalVoltage;
}

/*
 * File trailer for extendedKalmanFilter1RC.c
 *
 * [EOF]
 */
