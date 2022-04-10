void measureVoltage(double *voltage, double *current) {

  double R1 = 2.2, R2 =  3.3, R3 = 8.2, R4 = 6.8; 
  double sumCur = 0.0, sumVolt = 0.0, Vcc = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int numSamples = 250;
  int counter = 0;
  double currentTime = 0.0;

  //Get numSamples sumCur  
  while(counter < numSamples) {
    if(micros() >= currentTime + 200) {
      int meas = analogRead(A1);
      sumCur = sumCur + sq(meas);  //Add sumCur together
      sumVolt = sumVolt + analogRead(A0);  //Add sumVolt together
      currentTime = micros();
      counter = counter + 1;
    }
  }
  currentTime = 0.0;
  sumCur = sqrt(sumCur / numSamples) - 1.5; //Taking Average of sumCur
  sumVolt = sumVolt / numSamples; //Taking Average of sumCur
  
  //Vcc = analogRead(A2) * (Vref / 1023.0) * ((R1+R2)/R2);
  Vcc = analogRead(A4) * (Vref / 1023.0);
  *current = ((sumCur * (Vref / 1023.0)) - Vcc) / 0.00625;
  *voltage = (sumVolt * (Vref / 1023.0)) * ((R4+R3)/R3);

  //Serial.print(Vcc, 5); Serial.print("  Vcc;  ");
  
  if(Vcc < 0.01 || (*current < 0.09 && *current > -0.09)) {
    //*current = 0.0;
  }  
  
}


// Fonction qui mesure et affiche la température mesuré par le capteur, attention le capteur à les bornes positifs et négatifs inversés et il faut le connecter à du 3.3V
double mesureTemperature() {

  int R1 = 10000; // Valeur de la résistance mise en série avec la résistance qui permet de faire la mesure de température
  double R1c = 2.2, R2c =  3.3;
  double A = 0.000623, B = 0.000349, C = -0.0000005648; // Les coefficients de Steinhart-Hart obtenus expérimentalement pour notre thermistance
  double logR_th, T;
  double Vin = 3.3;
  double pas = Vin / 1023; // L'arduino pour ces mesures découpe 5V en 1024 valeurs discrètes
  
  int mesure_tension = analogRead(A3);  // Lis la tension en un format digital compris entre 0 et 1023 (découpe 5V en 1024 parts égales)
  double Vcc = analogRead(A2) * (Vin / 1023.0) * ((R1c+R2c)/R2c);
  double tension = mesure_tension * pas; // Pour obtenir la tension sous format analogique il faut la multiplier par le pas
  
  // Calcul de la résistance
  double R_th = R1 * ( (Vcc/tension) - 1);


  /* ############ Mesure de température ########################################## */ 
  logR_th = log(R_th);
  T = (1.0 / (A + B*logR_th + C*logR_th*logR_th*logR_th)); // Formule de Steinhart-Hart liant la résistance mesuré à la température (en Kelvin)
  T = T - 273.15; // Conversion Kelvin -> Celsius
  
  return T;
}
