int initializeSD()
{
  Serial.println("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin()) {
    Serial.println("SD card is ready to use.");
    return 1;
  } else {
    Serial.println("SD card initialization failed");
    return 0;
  }
}

int createFileWrite(const char *filename)
{
  file = SD.open(filename, FILE_WRITE);

  if(file)
  {
    Serial.println("File created successfully.");
    return 1;
  } else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}


int createFileRead(const char *filename)
{
  file = SD.open(filename);

  if(file)
  {
    Serial.println("File created successfully.");
    return 1;
  } else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}

int writeToFile(char *text)
{
  if (file) {
    file.println(text);
    Serial.println("Writing to file: ");
    Serial.println(text);
    return 1;
  } else {
    Serial.println("Couldn't write to file");
    return 0;
  }
}

int readFile(char *text) {
  if (file) {
    const char *res;
    while (file.available()) {
      res = file.readString().c_str();
    }
    sprintf(text, "\0");
    strcat(text, res);
    Serial.println("Reading file: ");
    Serial.println(res);
    return 1;
  } else {
    Serial.println("Couldn't read file");
    return 0;
  }
}

void closeFile()
{
  if(file)
  {
    file.close();
    Serial.println("File closed");
  }
}

void saveSD(const char *filename, double *X, double *Z, double *P_x, double *P_z, double *Q_x, double *Q_z, double *alpha_x, double *betha_x, double *alpha_z, double *betha_z, double *SOCOCV, double *dSOCOCV, double Qn_rated, double voltage_rated, double current_rated) {
  char input[800];
  sprintf(input, "{'C1': %0.2e, 'R0': %0.2e, 'R1': %0.2e, 'SOCOCV': [%0.2e, %0.2e, %0.2e, %0.2e, %0.2e], 'dSOCOCV': [%0.2e, %0.2e, %0.2e, %0.2e], 'SOC_init': %0.2e, 'P_x': [%0.2e, %0.2e], 'P_z': [%0.2e, %0.2e, %0.2e], 'Q_x': [%0.2e, %0.2e], 'Q_z': [%0.2e, %0.2e, %0.2e], 'alpha_x': %0.2e, 'betha_x': %0.2e, 'alpha_z': %0.2e, 'betha_z': %0.2e, 'Qn_rated': %0.2e, 'voltage_rated': %0.2e, 'current_rated': %0.2e}", Z[2]/Z[1], Z[0], Z[1], SOCOCV[0], SOCOCV[1], SOCOCV[2], SOCOCV[3], SOCOCV[4], dSOCOCV[0], dSOCOCV[1], dSOCOCV[2], dSOCOCV[3], X[0], P_x[0], P_x[3], P_z[0], P_z[4], P_z[8], Q_x[0], Q_x[3], Q_z[0], Q_z[4], Q_z[8], alpha_x[0], betha_x[0], alpha_z[0], betha_z[0], Qn_rated, voltage_rated, current_rated);
  SD.remove(filename);
  if(createFileWrite(filename)) {
      writeToFile(input);
      closeFile();
  }
}

void ReadSD(char *input, const char *filename) {
  if(createFileRead(filename)) {
      readFile(input);
      closeFile();
    }
}

void printSd(const char *filename, int nbBatteries, double *V, double *I, double *T, double *X, double VSP, int hours, int minutes, int seconds, int day, int month, int year) {
  char data[1000] = "\0";
  char res[500];
  sprintf(res, "{'I batteries': %0.2e, 'date':%d-%d-%d, 'batteries':[", I[0], day, month, year);
  strcat(data, res);
   for(int i = 0; i < nbBatteries; i++) {
      sprintf(res, "{'id':%d, 'S': %0.2e, 'V': %0.2e, 'T': %0.2e, 'P':%0.2e, 'H': %d:%d:%d},", i, X[3*i], V[i], T[i], I[0]*V[i], hours, minutes, seconds);
      strcat(data, res);
    }
    sprintf(res, "], 'solarPannels': [{'I':  %0.2e, 'V': %0.2e, 'P':  %0.2e}]}\"", I[1], VSP, I[1]*VSP);
    strcat(data, res);
    Serial.println("Save data to SD card :");
    Serial.println(data);
    if(createFileWrite(filename)) {
      writeToFile(data);
      closeFile();
  }
}
