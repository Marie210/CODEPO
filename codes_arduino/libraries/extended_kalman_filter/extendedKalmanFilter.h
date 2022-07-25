/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: extendedKalmanFilter.h
 *
 * MATLAB Coder version            : 5.1
 * C/C++ source code generated on  : 21-Jul-2022 14:32:40
 */

#ifndef EXTENDEDKALMANFILTER_H
#define EXTENDEDKALMANFILTER_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus

extern "C" {

#endif

  /* Function Declarations */
  extern void extendedKalmanFilter(double U, double X[3], double Z[5], const
    double SOCOCV[5], const double dSOCOCV[4], double Vt_Actual, double P_x[9],
    double P_z[25], const double Q_x[9], const double Q_z[25], double R_x,
    double R_z, double DeltaT, double Qn_rated, double *Error_x);

#ifdef __cplusplus

}
#endif
#endif

/*
 * File trailer for extendedKalmanFilter.h
 *
 * [EOF]
 */
