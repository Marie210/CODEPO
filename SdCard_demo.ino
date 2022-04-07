 // Tutoriel : http://educ8s.tv/arduino-sd-card-tutorial

#include <SD.h>
#include <SPI.h>

int CS_PIN = 10;

File file;

void setup()
{

  Serial.begin(9600);
  Serial.println("Start");
  
  initializeSD();
  createFile("test.txt");  // Fonction qui créée le fichier
  writeToFile("Eeet j'ai du côtoyé le paaaaaaavé");
  closeFile();

  Serial.println("Stop");
}

void loop()
{
}

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
