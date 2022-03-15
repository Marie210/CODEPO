#include <ArduinoJson.h>
#include <SD.h>

void initialisation(const String input, double X[3], double Z[5], 
              double SOCOCV[12], double dSOCOCV[11], double P_x[9], double P_z[25], double Q_x[9],
              double Q_z[25], double *R_x, double *R_z, double *DeltaT, double *Qn_rated) {
              double parameters[5];
              double SOC_init;
              // load data from JSON file
              loadJson(input, parameters, &SOC_init, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, R_x, R_z, DeltaT, Qn_rated);
              // create X and Z arrays from parameters
              X[0] = SOC_init; X[1] = 0.0; X[2] = 0.0;
              Z[0] = parameters[2]; Z[1] = parameters[3]; Z[2] = parameters[4];
              Z[3] = parameters[3] * parameters[0];
              Z[4] = parameters[4] * parameters[1];
}


// Loads the configuration from a file
void loadJson(String input, double parameters[5], double *SOC_init, double SOCOCV[12], double dSOCOCV[11], 
              double P_x[9], double P_z[25], double Q_x[9], double Q_z[25],
              double *R_x, double *R_z, double *DeltaT, double *Qn_rated) {
  
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
  *DeltaT = doc["DeltaT"];
  *Qn_rated = doc["Qn_rated"];
  
  for (int i = 0; i < 12; i++) {
    SOCOCV[i] = doc["SOCOCV"][i];
  }

  for (int i = 0; i < 11; i++) {
    dSOCOCV[i] = doc["dSOCOCV"][i];
    P_x[i] = doc["P_x"][i];
    Q_x[i] = doc["Q_x"][i];
  }

  for (int i = 0; i < 25; i++) {
    P_z[i] = doc["P_z"][i];
    Q_z[i] = doc["Q_z"][i];
  }
  

}
