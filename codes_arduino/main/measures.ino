double measureVoltage(double R1, double R2, int numSamples, const byte VPin) {
  
  double sumVolt = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int counter = 0;
  double currentTime = 0.0;
  double V = 0.0;
  //Get numSamples sumCur  
  while(counter < numSamples) {
    if(micros() >= currentTime + 200) {
      sumVolt = sumVolt + analogRead(VPin);  // Add sumVolt together
      currentTime = micros();
      counter = counter + 1;
    }
  }
  sumVolt = sumVolt / numSamples; //Taking Average of sumCur
  V = (sumVolt * (Vref / 1023.0)) * ((R2+R1)/R2);
  if(V < 0.3 && V > -0.3) {
    V = 0.0;
  }  
  return V;
}


double measureCurrent(const byte pinI, const byte VccIPin, double mvPerI, int numSamples) {
  double sumCur = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int counter = 0;
  double currentTime = 0.0;
  double offset = 0.0;
  double I = 0.0;
  //Get numSamples sumCur  
  while(counter < numSamples) {
    if(micros() >= currentTime + 200) {
      int meas = analogRead(pinI);
      sumCur = sumCur + sq(meas);  // Add sumCur together
      currentTime = micros();
      counter = counter + 1;
    }
  }
  sumCur = sqrt(sumCur / numSamples) - offset; //Taking Average of sumCur
  double Vcc = analogRead(VccIPin) * (Vref / 1023.0);
  I = ((sumCur * (Vref / 1023.0)) - Vcc) / mvPerI;
  // application d'un treshold sur le courant
  if(Vcc < 0.01 || (I < 0.3 && I > -0.3)) {
    //I = 0.0;
  }  
  Serial.print("I = "); Serial.println(I);
  return I;
}


// Fonction qui mesure et affiche la température mesuré par le capteur, attention le capteur à les bornes positifs et négatifs inversés et il faut le connecter à du 3.3V
double mesureTemperature(const byte TPin, const byte VccPin, double R, double RVcc1, double RVcc2) {

  double A = 0.000623, B = 0.000349, C = -0.0000005648; // Les coefficients de Steinhart-Hart obtenus expérimentalement pour notre thermistance
  double logR_th, T;
  double Vin = 3.3;
  double pas = Vin / 1023; // L'arduino pour ces mesures découpe 3.3V en 1023 valeurs discrètes
  
  int mesure_tension = analogRead(TPin);  // Lis la tension en un format digital compris entre 0 et 1023 (découpe 3.3V en 1024 parts égales)
  Serial.println(mesure_tension);
  double Vcc = analogRead(VccPin) * (Vin / 1023.0) * ((RVcc1+RVcc2)/RVcc2);
  Serial.println(Vcc);
  double tension = mesure_tension * pas; // Pour obtenir la tension sous format analogique il faut la multiplier par le pas
  
  // Calcul de la résistance
  if(tension > -0.3 && tension < 0.3) {
    T = 0.0;
  } else {
    double R_th = R * ( (Vcc/tension) - 1);
    // Mesure de température
    logR_th = log(R_th);
    T = (1.0 / (A + B*logR_th + C*logR_th*logR_th*logR_th)); // Formule de Steinhart-Hart liant la résistance mesuré à la température (en Kelvin)
    T = T - 273.15; // Conversion Kelvin -> Celsius
  }
  
  return T;
}

double readAnalogMux(int channel, int PIN_ADDR_A, int PIN_ADDR_B, int PIN_ADDR_C) {
  // On sélectionne la voie
  digitalWrite(PIN_ADDR_A, bitRead(channel, 0));
  digitalWrite(PIN_ADDR_B, bitRead(channel, 1));
  digitalWrite(PIN_ADDR_C, bitRead(channel, 2));
}

void takeMeasures(double *V, double *I, double *T, int nbBatteries, int nbCurrent, int PIN_ADDR_A, int PIN_ADDR_B, int PIN_ADDR_C, double RV, double *pot, int numSamples, const byte VPin, const byte pinI, const byte VccIPin, double pinISP, double VccIPinSP, double mvPerI, const byte TPin, const byte VccPin, double R, double RVcc1, double RVcc2) {
  for(int i = 0; i < nbBatteries; i++){
    readAnalogMux(i, PIN_ADDR_A, PIN_ADDR_B, PIN_ADDR_C);
    V[i] = measureVoltage(RV, pot[i], numSamples, VPin);
    T[i] = mesureTemperature(TPin, VccPin, R, RVcc1, RVcc2);
    Serial.print(i); Serial.print(" = "); Serial.println(T[i]);
    if(i < nbCurrent) {
      if(i == 0) {
        I[i] = measureCurrent(pinI, VccIPin, mvPerI, numSamples);
      } else {
        I[i] = measureCurrent(pinISP, VccIPinSP, mvPerI, numSamples);
      }
    }
  }
}

uint32_t ticks_diff(uint32_t t0, uint32_t t1) {
  return ((t0 < t1) ? 84000 + t0 : t0) - t1;
}
