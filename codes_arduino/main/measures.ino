
void measureVoltage(double *voltage, double *current) {

  double R1 = 2.2, R2 =  3.3, R3 = 8.2, R4 = 6.8; 
  double sumCur = 0.0, sumVolt = 0.0, Vcc = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int numSamples = 250;

  //Get numSamples sumCur  
  for (int x = 0; x < numSamples; x++) {
    sumCur = sumCur + analogRead(A0);  //Add sumCur together
    sumVolt = sumVolt + analogRead(A1);  //Add sumVolt together
    delay(2); // let ADC settle before next sample 3ms
  }
  sumCur = sumCur / numSamples; //Taking Average of sumCur
  sumVolt = sumVolt / numSamples; //Taking Average of sumCur
  
  Vcc = analogRead(A2) * (Vref / 1023.0) * ((R1+R2)/R2);
  *current = ((sumCur * (Vref / 1023.0)) - Vcc/2)/0.185;
  *voltage = (sumVolt * (Vref / 1023.0)) * ((R4+R3)/R3);

  if(Vcc < 0.01 || (*current < 0.2 && *current > -0.2)) {
    *current = 0.0;
  }  
}


// Fonction qui mesure et affiche la température mesuré par le capteur, attention le capteur à les bornes positifs et négatifs inversés et il faut le connecter à du 3.3V
double mesureTemperature() {

  int R1 = 10000; // Valeur de la résistance avec laquelle on fait la mesure de la température
  double c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
  double logR2, R2, T;
  int mesure_thermistor;
  
  mesure_thermistor = analogRead(A3);  // Prise de mesure
    
  R2 = R1 * (1023.0 / (double)mesure_thermistor - 1.0); //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  T = T - 273.15; //convert Kelvin to Celcius

  return T;
}
