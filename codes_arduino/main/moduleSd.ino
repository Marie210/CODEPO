void initializeSD()
{
  Serial.println("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
}

int createFile(char filename[])
{
  file = SD.open(filename, FILE_WRITE);

  if (file)
  {
    Serial.println("File created successfully.");
    return 1;
  } else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}

int writeToFile(char text[])
{
  if (file)
  {
    file.println(text);
    Serial.println("Writing to file: ");
    Serial.println(text);
    return 1;
  } else
  {
    Serial.println("Couldn't write to file");
    return 0;
  }
}

void closeFile()
{
  if (file)
  {
    file.close();
    Serial.println("File closed");
  }
}

void printSd(double Vt_Actual, double U, double sec, double temperature) {
    char data[150];
    sprintf(data, "%f     VT;     %f     I;     %f     Ti;     %f     Te;", Vt_Actual, U, sec, temperature);
    if(createFile("data.txt")) {
      writeToFile(data);
      closeFile();
    }
}

void printSd2(double Vt_Actual, double U, double sec, double temperature, double SOC) {
    char data[150];
    sprintf(data, "%f     VT;     %f     I;     %f     Ti;     %f     Te;     %f     S;", Vt_Actual, U, sec, temperature, SOC);
    if(createFile("data.txt")) {
      writeToFile(data);
      closeFile();
    }
}
