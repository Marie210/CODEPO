
void measureVoltage(double *voltage, double *current) {

  double R1 = 2.2, R2 =  3.3, R3 = 8.2, R4 = 6.8; 
  double sumCur = 0.0, sumVolt = 0.0, Vcc = 0.0;
  double Vref = 3.3; // tension de reference de l'arduino
  int numSamples = 150;

  //Get numSamples sumCur  
  for (int x = 0; x < numSamples; x++) {
    sumCur = sumCur + analogRead(A0);  //Add sumCur together
    sumVolt = sumVolt + analogRead(A2);  //Add sumVolt together
    delay(3); // let ADC settle before next sample 3ms
  }
  sumCur = sumCur / numSamples; //Taking Average of sumCur
  sumVolt = sumVolt / numSamples; //Taking Average of sumCur
  
  Vcc = analogRead(A2) * (Vref / 1023.0) * ((R1+R2)/R2);
  *current = ((sumCur * (Vref / 1023.0)) - Vcc/2)/0.185;
  *voltage = (sumVolt * (Vref / 1023.0)) * ((R4+R3)/R3);

  if(Vcc < 0.01 || (*current < 0.1 && *current > -0.1)) {
    *current = 0.0;
  }
  Serial.print(Vcc, 5); Serial.print("  Vcc;  ");
  
}
