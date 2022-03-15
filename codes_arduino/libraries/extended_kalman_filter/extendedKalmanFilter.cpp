/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: extendedKalmanFilter.c
 *
 * MATLAB Coder version            : 5.1
 * C/C++ source code generated on  : 15-Feb-2022 16:12:22
 */

/* Include Files */
#include "extendedKalmanFilter.h"
#include <math.h>
#include <string.h>
#include <Arduino.h>

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
 * Arguments    : double U
 *                double X[3]
 *                double Z[5]
 *                const double SOCOCV[12]
 *                const double dSOCOCV[11]
 *                double Vt_Actual
 *                double P_x[9]
 *                double P_z[25]
 *                const double Q_x[9]
 *                const double Q_z[25]
 *                double R_x
 *                double R_z
 *                double DeltaT
 *                double Qn_rated
 * Return Type  : double
 */
double extendedKalmanFilter(double U, double X[3], double Z[5], const double
  SOCOCV[12], const double dSOCOCV[11], double Vt_Actual, double P_x[9], double
  P_z[25], const double Q_x[9], const double Q_z[25], double R_x, double R_z,
  double DeltaT, double Qn_rated)
{
  double c_I[25];
  double d_I[25];
  double dv[15];
  double A[9];
  double b_A[9];
  double C_z[5];
  double KalmanGain_z[5];
  double C_x[3];
  double KalmanGain_x[3];
  double c_A[3];
  double Error_x;
  double OCV;
  double SOC;
  double a1_tmp;
  double a2_tmp;
  double d;
  double d1;
  double dOCV;
  int i;
  int i1;
  int k;
  signed char b_I[25];
    
    
  /*  States */
  SOC = X[0];

  /*  Parameters */
  /*  Coef in A : Fk : State transition matrix */
  a1_tmp = exp(-DeltaT / Z[3]);
  a2_tmp = exp(-DeltaT / Z[4]);

  /*  Coef in B : Gk : Input control matrix */
  /*  Coef in mat : dXk/dThetak */
  /*  Linearizing the model by linearizing the C matrix */
  dOCV = dSOCOCV[0];
  for (k = 0; k < 10; k++) {
    dOCV = SOC * dOCV + dSOCOCV[k + 1];
  }

  /*  dUoc(SOC)/dSOC at SOC = SOCk */
  C_x[0] = dOCV;
  C_x[1] = -1.0;
  C_x[2] = -1.0;

  /*  Hx : Jacobian measurement matrix of x */
  /*  dXk/dThetak */
  dv[1] = 0.0;
  d = DeltaT / (Z[3] * Z[3]);
  dv[4] = -U * (exp(d) - 1.0);
  dv[7] = 0.0;
  dv[10] = d * (X[1] - Z[1] * U) * exp(DeltaT / Z[3]);
  dv[13] = 0.0;
  dv[2] = 0.0;
  dv[5] = 0.0;
  d = DeltaT / (Z[4] * Z[4]);
  dv[8] = -U * (exp(d) - 1.0);
  dv[11] = 0.0;
  dv[14] = d * (X[2] - Z[2] * U) * exp(DeltaT / Z[4]);
  for (i = 0; i < 5; i++) {
    dv[3 * i] = 0.0;
    KalmanGain_z[i] = (dOCV * 0.0 + -dv[3 * i + 1]) + -dv[3 * i + 2];
  }

  C_z[0] = -U + KalmanGain_z[0];
  C_z[1] = KalmanGain_z[1];
  C_z[2] = KalmanGain_z[2];
  C_z[3] = KalmanGain_z[3];
  C_z[4] = KalmanGain_z[4];

  /*  Htheta : Jacobian measurement matrix of theta */
  /*  Knowing SOC, we can compute OCV thanks to the relationship SOC-OCV */
  OCV = SOCOCV[0];
  for (k = 0; k < 11; k++) {
    OCV = SOC * OCV + SOCOCV[k + 1];
  }

  /*  calculate the values of OCV at the given SOC, using the polynomial SOCOCV */
  /*  Terminal voltage estimation */
  /*  Calculate the Vt error */
  Error_x = Vt_Actual - (((OCV - Z[0] * U) - X[1]) - X[2]);
    //Serial.print(" ---> C code -- V = ");
    Serial.print((((OCV - Z[0] * U) - X[1]) - X[2]), 5); Serial.print("  CV;  ");

  /*  --- EKF ALGORITHM --- */
  /*  --> SOC ESTIMATION */
  /*  Fk : State transition matrix */
  A[0] = 1.0;
  A[3] = 0.0;
  A[6] = 0.0;
  A[1] = 0.0;
  A[4] = a1_tmp;
  A[7] = 0.0;
  A[2] = 0.0;
  A[5] = 0.0;
  A[8] = a2_tmp;

  /*  Gk : Input control matrix */
  /*  Prediction */
  for (i = 0; i < 3; i++) {
    i1 = (int)A[i];
    d = A[i + 3];
    SOC = A[i + 6];
    for (k = 0; k < 3; k++) {
      b_A[i + 3 * k] = ((double)i1 * P_x[3 * k] + d * P_x[3 * k + 1]) + SOC *
        P_x[3 * k + 2];
    }
  }

  for (i = 0; i < 3; i++) {
    d = b_A[i];
    SOC = b_A[i + 3];
    d1 = b_A[i + 6];
    for (i1 = 0; i1 < 3; i1++) {
      k = i + 3 * i1;
      P_x[k] = ((d * A[i1] + SOC * A[i1 + 3]) + d1 * A[i1 + 6]) + Q_x[k];
    }
  }

  /*  Update */
  OCV = 0.0;
  for (i = 0; i < 3; i++) {
    KalmanGain_x[i] = (P_x[i] * dOCV + -P_x[i + 3]) + -P_x[i + 6];
    OCV += ((dOCV * P_x[3 * i] + -P_x[3 * i + 1]) + -P_x[3 * i + 2]) * C_x[i];
  }

  OCV = 1.0 / (OCV + R_x);
  for (i = 0; i < 3; i++) {
    KalmanGain_x[i] *= OCV;
    c_A[i] = (A[i] * X[0] + A[i + 3] * X[1]) + A[i + 6] * X[2];
  }

  X[0] = -(DeltaT / Qn_rated) * U;
  X[1] = Z[1] * (1.0 - a1_tmp) * U;
  X[2] = Z[2] * (1.0 - a2_tmp) * U;
  X[0] = (c_A[0] + X[0]) + KalmanGain_x[0] * Error_x;
  X[1] = (c_A[1] + X[1]) + KalmanGain_x[1] * Error_x;
  X[2] = (c_A[2] + X[2]) + KalmanGain_x[2] * Error_x;
  memset(&A[0], 0, 9U * sizeof(double));
  A[0] = 1.0;
  A[4] = 1.0;
  A[8] = 1.0;
  d = KalmanGain_x[0];
  SOC = KalmanGain_x[1];
  d1 = KalmanGain_x[2];
  for (i = 0; i < 3; i++) {
    OCV = C_x[i];
    b_A[3 * i] = A[3 * i] - d * OCV;
    k = 3 * i + 1;
    b_A[k] = A[k] - SOC * OCV;
    k = 3 * i + 2;
    b_A[k] = A[k] - d1 * OCV;
  }

  for (i = 0; i < 3; i++) {
    d = b_A[i];
    SOC = b_A[i + 3];
    d1 = b_A[i + 6];
    for (i1 = 0; i1 < 3; i1++) {
      A[i + 3 * i1] = (d * P_x[3 * i1] + SOC * P_x[3 * i1 + 1]) + d1 * P_x[3 *
        i1 + 2];
    }
  }

  memcpy(&P_x[0], &A[0], 9U * sizeof(double));

  /*  --> BATTERY PARAMETERS ESTIMATION */
  /*  Prediction */
  for (i = 0; i < 25; i++) {
    P_z[i] += Q_z[i];
  }

  /*  Update */
  OCV = 0.0;
  for (i = 0; i < 5; i++) {
    d = 0.0;
    for (i1 = 0; i1 < 5; i1++) {
      d += C_z[i1] * P_z[i1 + 5 * i];
    }

    OCV += d * C_z[i];
  }

  OCV = 1.0 / (OCV + R_z);
  for (i = 0; i < 5; i++) {
    d = 0.0;
    for (i1 = 0; i1 < 5; i1++) {
      d += P_z[i + 5 * i1] * C_z[i1];
    }

    d *= OCV;
    KalmanGain_z[i] = d;
    Z[i] += d * Error_x;
  }

  for (i = 0; i < 25; i++) {
    b_I[i] = 0;
  }

  for (k = 0; k < 5; k++) {
    b_I[k + 5 * k] = 1;
  }

  for (i = 0; i < 5; i++) {
    for (i1 = 0; i1 < 5; i1++) {
      k = i1 + 5 * i;
      d_I[k] = (double)b_I[k] - KalmanGain_z[i1] * C_z[i];
    }
  }

  for (i = 0; i < 5; i++) {
    for (i1 = 0; i1 < 5; i1++) {
      d = 0.0;
      for (k = 0; k < 5; k++) {
        d += d_I[i + 5 * k] * P_z[k + 5 * i1];
      }

      c_I[i + 5 * i1] = d;
    }
  }

  memcpy(&P_z[0], &c_I[0], 25U * sizeof(double));
    //Serial.print(" ---> C code -- SOC = ");
    //Serial.println(X[0]);
  return Error_x;
}

/*
 * File trailer for extendedKalmanFilter.c
 *
 * [EOF]
 */
