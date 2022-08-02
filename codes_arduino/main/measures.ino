double measureVoltage(double R1, double R2, int nbSamples, const byte VPin) {
  double sumVolt = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int counter = 0;
  double currentTime = 0.0;
  double V = 0.0;
  //Get nbSamples sumCur  
  while(counter < nbSamples) {
    if(micros() >= currentTime + 200) {
      sumVolt = sumVolt + analogRead(VPin);  // Add sumVolt together
      currentTime = micros();
      counter = counter + 1;
    }
  }
  sumVolt = sumVolt / nbSamples; //Taking Average of sumCur
  V = (sumVolt * (Vref / 1023.0)) * ((R2+R1)/R2);
  if(V < 0.3 && V > -0.3) {
    V = 0.0;
  }
  return V;
}


double measureCurrent(int nbSamples, double offset, double mvPerI, int pinCurrent, int pinVcc) {
  double sumCur = 0.0;
  double Vref = 3300; // tension de reference de l'arduino
  int counter = 0;
  double currentTime = 0.0;
  double I = 0.0;
  //Get nbSamples sumCur  
  while(counter < nbSamples) {
    if(micros() >= currentTime + 200) {
      int meas = analogRead(pinCurrent)-analogRead(pinVcc);
      sumCur = sumCur + meas;  // Add sumCur together
      currentTime = micros();
      counter = counter + 1;
    }
  }
  sumCur = sumCur / nbSamples; //Taking Average of sumCur
  I = (sumCur * (Vref / 1023.0)) / mvPerI - offset;
  // Application d'un treshold sur le courant
  if(I < 0.3 && I > -0.3) {
    I = 0.0;
  }  
  return I;
}


// Fonction qui mesure et affiche la température mesuré par le capteur, attention le capteur à les bornes positifs et négatifs inversés et il faut le connecter à du 3.3V
double mesureTemperature(const byte TPin, const byte VccPin, double R, double RT1, double RT2, double *TH) {

  double A = TH[0], B = TH[1], C = TH[2]; // Les coefficients de Steinhart-Hart obtenus expérimentalement pour notre thermistance
  double logR_th, T;
  double Vin = 3.3;
  double pas = Vin / 1023; // L'arduino pour ces mesures découpe 3.3V en 1023 valeurs discrètes
  
  int mesure_tension = analogRead(TPin);  // Lis la tension en un format digital compris entre 0 et 1023 (découpe 3.3V en 1024 parts égales)
  double Vcc = analogRead(VccPin) * pas * ((RT1+RT2)/RT2);
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

void takeMeasures(double *V, double *I, double *T, double *VSP, int nbBatteries, int nbCurrent, double RV, double *pot, int nbSamples, double R, double RT1, double RT2, double RSP1, double RSP2, double offset_20, double offset_100, double mvPerI_20, double mvPerI_100, double *PMean, double *PSPMean, double *VMean, double *IMean, double *TMean, double *VSPMean, int *counterMean, double *X, double *SMean, double *TH) {
  for(int i = 0; i < nbBatteries; i++){
    readAnalogMux(i, PIN_ADDR_A, PIN_ADDR_B, PIN_ADDR_C);
    V[i] = measureVoltage(RV, pot[i], nbSamples, VPin);
    T[i] = mesureTemperature(TPin, VccPin, R, RT1, RT2, TH+3*i);
    if(i < nbCurrent) {
      if(i == 0) {
        I[i] = measureCurrent(nbSamples, offset_20, mvPerI_20, pinI, VccIPin);
      } else {
        I[i] = measureCurrent(nbSamples, offset_100, mvPerI_100, pinISP, VccIPinSP);
      }
      IMean[i] += I[i];
    }
    VMean[i] += V[i];
    TMean[i] += T[i];
    SMean[i] += X[2*i];
    PMean[i] += V[i]*I[0];
  }
  *VSP = measureVoltage(RSP1, RSP2, nbSamples, VSPPin);
  *VSPMean += *VSP;
  *PSPMean += (*VSP)*I[1];
  *counterMean += 1;
}

void updateMeasures(int timeHour, int nbBatteries, int *Hcounter, int *HSPcounter, double *normH, double *normHSP, double *listheureBat, double *listheureSP, double *VMean, double *IMean, double *SMean, double *TMean, double *PMean, double *VSPMean, double *PSPMean, int *counterMean, double *V, double *I, double *X, double VSP, double *VH, double *IH, double *SH, double *PH, double *PSPH, double *VSPH, double *ISPH, int *hourDaySP, int *minuteDaySP, int *hourDay, int *minuteDay, int *day, int *month, int *year) {
  if(timeHour == listheureBat[*Hcounter] && checkH[*Hcounter] == false) {
    checkH[*Hcounter] = true;
    hourDay[*Hcounter] = rtc.getHours();
    minuteDay[*Hcounter] = rtc.getMinutes();
    if(*Hcounter < 4) {
      if(*normH == 0) { *normH = 1; }
      for(int i = 0; i < nbBatteries; i++) {
        VH[5*i + *Hcounter] = VH[5*i + *Hcounter]/ *normH;
        IH[5*i + *Hcounter] = IH[5*i + *Hcounter]/ *normH;
        SH[5*i + *Hcounter] = SH[5*i + *Hcounter]/ *normH;
        PH[5*i + *Hcounter] = PH[5*i + *Hcounter]/ *normH;
      }
      *normH = 0;
      *Hcounter += 1;
    }
  }
  if(timeHour == listheureSP[*HSPcounter] && checkSPH[*HSPcounter] == false) {
    checkSPH[*HSPcounter] = true;
    hourDaySP[*HSPcounter] = rtc.getHours();
    minuteDaySP[*HSPcounter] = rtc.getMinutes();
    if(*normHSP == 0) { *normHSP = 1; }
    PSPH[*HSPcounter] = PSPH[*HSPcounter]/ *normHSP;
    VSPH[*HSPcounter] = VSPH[*HSPcounter]/ *normHSP;
    ISPH[*HSPcounter] = ISPH[*HSPcounter]/ *normHSP;
    *normHSP = 0;
    if(*HSPcounter < 8) {
      *HSPcounter += 1;
    }
  }
  if(*Hcounter == 4 && *HSPcounter == 8) {
    if(*normH == 0) { *normH = 1; }
    for(int i = 0; i < nbBatteries; i++) {
      VH[5*i + *Hcounter] = VH[5*i + *Hcounter]/ *normH;
      IH[5*i + *Hcounter] = IH[5*i + *Hcounter]/ *normH;
      SH[5*i + *Hcounter] = SH[5*i + *Hcounter]/ *normH;
      PH[5*i + *Hcounter] = PH[5*i + *Hcounter]/ *normH;
    }
    *normH = 0;
    sendMeasures(day, month, year, VMean, IMean, SMean, TMean, PMean, *VSPMean, *PSPMean, nbBatteries, VH, IH, SH, PH, PSPH, VSPH, ISPH, hourDay, minuteDay);
    reInitialiseMeasures(VH, IH, SH, PH, PSPH, VSPH, ISPH, VSPMean, PSPMean, Hcounter, HSPcounter, counterMean, nbBatteries);
  }
  if(timeHour >= listheureBat[*Hcounter-1] && timeHour < listheureBat[*Hcounter-1] + 1) {
    for(int i = 0; i < nbBatteries; i++) {
      VH[5*i + *Hcounter] += V[i];
      IH[5*i + *Hcounter] += I[0];
      SH[5*i + *Hcounter] += X[2*i];
      PH[5*i + *Hcounter] += V[i]*I[0];
    }
    *normH += 1;
  }
  if(timeHour >= listheureSP[*HSPcounter-1] && timeHour < listheureSP[*HSPcounter-1] + 1) {
    PSPH[*HSPcounter] += VSP*I[1];
    VSPH[*HSPcounter] += VSP;
    ISPH[*HSPcounter] += I[1];
    *normHSP += 1;
  }
}

void reInitialiseMeasures(double *VH, double *IH, double *SH, double *PH, double *PSPH,  double *VSPH, double *ISPH, double *VSPMean, double *PSPMean, int *Hcounter, int *HSPcounter, int *counterMean, int nbBatteries) {
  for(int i = 0; i < *Hcounter * nbBatteries; i++) {
    VH[i] = 0;
    IH[i] = 0;
    SH[i] = 0;
    PH[i] = 0;
    if(i < nbBatteries) {
      VMean[i] = 0;
      IMean[i] = 0;
      SMean[i] = 0;
      TMean[i] = 0;
      PMean[i] = 0;
    }
    if(i < *HSPcounter) {
      PSPH[i] = 0;
    }
    *VSPMean = 0;
    *PSPMean = 0;
    *Hcounter = 0;
    *HSPcounter = 0;
  }
  *counterMean = 0;
}

void sendMeasures(int *day, int *month, int *year, double *VMean, double *IMean, double *SMean, double *TMean, double *PMean, double VSPMean, double PSPMean, int nbBatteries, double *VH, double *IH, double *SH, double *PH, double *PSPH, double *VSPH, double *ISPH, int *hourDay, int *minuteDay) {
  *day = rtc.getDay();
  *month = rtc.getMonth();
  *year = rtc.getYear();
  for(int i = 0; i < nbBatteries; i++) {
    VMean[i] /= (double)counterMean;
    IMean[i] /= (double)counterMean;
    SMean[i] /= (double)counterMean;
    TMean[i] /= (double)counterMean;
    PMean[i] /= (double)counterMean;
  }
  publish(nbBatteries, VH, IH, SH, PH, PSPH, VSPH, ISPH, hourDay, minuteDay, hourDaySP, minuteDaySP, *day, *month, *year, VMean, IMean, SMean, TMean, VSPMean/counterMean, PMean, PSPMean/counterMean);
}

uint32_t ticks_diff(uint32_t t0, uint32_t t1) {
  return ((t0 < t1) ? 84000 + t0 : t0) - t1;
}
