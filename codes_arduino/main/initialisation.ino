#include <ArduinoJson.h>
#include <SD.h>

void initialisation(const String input, double *X, double *Z, 
              double SOCOCV[12], double dSOCOCV[11], double *P_x, double *P_z, double *Q_x,
              double *Q_z, double *R_x, double *R_z, double *Qn_rated, double *voltage_rated, double *current_rated) {
              double parameters[5];
              double SOC_init;
              // load data from JSON file
              loadJson(input, parameters, &SOC_init, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, R_x, R_z, Qn_rated, voltage_rated, current_rated);
              // create X and Z arrays from parameters
              X[0] = SOC_init; X[1] = 0.0; X[2] = 0.0;
              Z[0] = parameters[2]; Z[1] = parameters[3]; Z[2] = parameters[4];
              Z[3] = parameters[3] * parameters[0];
              Z[4] = parameters[4] * parameters[1];
}


// Loads the configuration from a file
void loadJson(String input, double *parameters, double *SOC_init, double SOCOCV[12], double dSOCOCV[11], 
              double *P_x, double *P_z, double *Q_x, double *Q_z,
              double *R_x, double *R_z, double *Qn_rated, double *voltage_rated, double *current_rated) {
  
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  StaticJsonDocument<3500> doc;

  input.replace("'", "\"");
  

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, String(input));
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
    Serial.println(error.f_str());
  }

  // Copy values from the JsonDocument to the parameters structure
  parameters[0] = doc["C1"];
  parameters[1] = doc["C2"];
  parameters[2] = doc["R0"];
  parameters[3] = doc["R1"];
  parameters[4] = doc["R2"];
  
  
  *SOC_init = doc["SOC_init"];
  *R_x = doc["R_x"];
  *R_z = doc["R_z"];
  *Qn_rated = doc["Qn_rated"]; *Qn_rated = *Qn_rated * 3600.0;
  *voltage_rated = doc["voltage_rated"];
  *current_rated = doc["current_rated"];
  
  for (int i = 0; i < 12; i++) {
    SOCOCV[i] = doc["SOCOCV"][i];
  }

  for (int i = 0; i < 11; i++) {
    if(i < 9) {
        if(i%4 == 0) {
          P_x[i] = doc["P_x"];
          Q_x[i] = doc["Q_x"];
        } else {
          P_x[i] = 0;
          Q_x[i] = 0;
        }
    }
    dSOCOCV[i] = doc["dSOCOCV"][i];
  }

  for (int i = 0; i < 25; i++) {
    if(i%6 == 0) {
      P_z[i] = doc["P_z"];
      Q_z[i] = doc["Q_z"];
    } else {
      P_z[i] = 0;
      Q_z[i] = 0;
    }
  }
}

void duplicate(double *X, double *Z, double *P_x, double *P_z, double *Q_x, 
              double *Q_z, double *R_x, double *R_z, int nbBatteries) {
                
            for(int i = 1; i < nbBatteries; i++) {
              for (int j = 0; j < 9; j++) {
                Q_x[i*9 + j] = Q_x[j];
                P_x[i*9 + j] = P_x[j];
              }
              for (int j = 0; j < 5; j++) {
                Z[i*5 + j] = Z[j];
              }
              for (int j = 0; j < 3; j++) {
                X[i*3 + j] = X[j];
              }
              for (int j = 0; j < 25; j++) {
                P_z[i*25 + j] = P_z[j];
                Q_z[i*25 + j] = Q_z[j];
              }
              R_x[i] = R_x[0];
              R_z[i] = R_z[0];
            }
}

void copy(double* src, double* dst, int len, int index1, int index2) {
    for (int i = 0; i < len; i++) {
        dst[index1+i] = src[index2+i];
    }
}

void kalmanFilter(double I, double *X, double *Z, double *SOCOCV, double *dSOCOCV, double *V, double *P_x, double *P_z, double *Q_x, double *Q_z, double *R_x, double *R_z, double DeltaT, double Qn_rated, int nbBatteries) {
  for(int i = 0; i < nbBatteries; i++) {
    double Xb[3], Zb[5], P_xb[9], P_zb[25], Q_xb[9], Q_zb[25];
    copy(X, Xb, 3, 0, i*3);
    copy(Z, Zb, 5, 0, i*5);
    copy(P_x, P_xb, 9, 0, i*9);
    copy(P_z, P_zb, 25, 0, i*25);
    copy(Q_x, Q_xb, 9, 0, i*9);
    copy(Q_z, Q_zb, 25, 0, i*25);
    double R_xb = R_x[i];
    double R_zb = R_z[i];  
    error = extendedKalmanFilter(I, Xb, Zb, SOCOCV, dSOCOCV, V[i], P_xb, P_zb, Q_xb, Q_zb, R_xb, R_zb, DeltaT, Qn_rated);
    copy(Xb, X, 3, i*3, 0);
    copy(Zb, Z, 5, i*5, 0);
    copy(P_xb, P_x, 9, i*9, 0);
    copy(P_zb, P_z, 25, i*25, 0);
    copy(Q_xb, Q_x, 9, i*9, 0);
    copy(Q_zb, Q_z, 25, i*25, 0);
    R_x[i] = R_xb;
    R_z[i] = R_zb;
  }
}
