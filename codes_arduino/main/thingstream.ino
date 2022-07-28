int initThingstream(int *flag_init, int resetThingstreamPin) {
  if (*flag_init == 0 ){
    // reset the Thingstream click
    digitalWrite(resetThingstreamPin, LOW);
    delay(1200);
    digitalWrite(resetThingstreamPin, HIGH);
    Serial.println("DEBUG");
    Serial1.println("AT+IOTDEBUG=0");
    if(checkReception() == 1) { 
      *flag_init = 1; 
        messageLCD("Debug : success");
      } else {
        messageLCD("Debug : fail");
      }
  }
  else if(*flag_init == 1){
    Serial.println("CREATE");
    Serial1.println("AT+IOTCREATE");  
    if(checkReception() == 1) { 
        *flag_init = 2; 
        messageLCD("Create : success");
     } else {
        messageLCD("Create : fail");
     }
  }
  else if(*flag_init == 2){
    Serial.println("CONNECT");
    Serial1.println("AT+IOTCONNECT=true"); 
    if(checkReception() == 1) { 
        *flag_init = 3; 
        messageLCD("Connect : success");
    } else {
        messageLCD("Connect : fail");
    }
  }
  else if(*flag_init == 3) {
    Serial.println("SUBSCRIBE");
    Serial1.println("AT+IOTSUBSCRIBE=\"TEST1\",1");
    if(checkReception() == 1) { 
        *flag_init = 4; 
        messageLCD("Subscribe : success");
    } else {
        messageLCD("Subscribe : fail");
    }
  }
}

int checkReception() {
  unsigned long timeInit = millis()*0.001;
  char message[150];
  int count = 0;
  
  while(true) {
    // If maximum time (20s) to wait for message reception exceeded
    if(millis()*0.001 - timeInit > 20.0) { 
      Serial.println("To Long Time");
      return 0;
     }
    // If not yet exceeded
    if (Serial1.available()){
        char st = Serial1.read();
        message[count] = st;  
        message[count+1] = '\0';  
        count += 1; 
                
        if(message[count-1] == '\n'){
          Serial.print(String(message));  
          if (analyse(message) ==  1) {
            return 1;
          } else {
            return 0;
          }
        }
     }
  }
  
}

int readCommand(String *res) {
  bool check = true, start = false;
  unsigned long timeInit = millis()*0.001;
  char message[150];
  int count = 0;
 
  while(check) {
    
    if(millis()*0.001 - timeInit > 20.0) { 
      Serial.println("To Long Time to read command");  
      return 0;
     }
    
     if (Serial1.available()) {
        char st = Serial1.read();
        
        if(start == false) {
          start = true;
          if(st != '*') {
            check = false;
            return 0;
          }
        } else {
            if(st == '*') {
                *res = String(message);
                Serial.print("reception message : ");
                Serial.print(*res);  
                Serial.print("first letter : ");
                Serial.println(message[0]);
                return 1;
            } else {
                message[count] = st;  
                message[count+1] = '\0';  
                count += 1; 
            }
        }
     } else if(start == false) {
          check = false;
          return 0;
     }
  }
}


int analyse (String st) {
  // Arret de la publication si le mot STOP est reçu par le Click
  int res = 1;  
  if(st.indexOf("GO") > 0 || st.indexOf("SUCCESS") > 0) {
    res = 1;
  } else {
    res = 0;
  }
  return res; 
}


void publish(int nbBatteries, double *VH, double *IH, double *SH, double *PH, double *PSPH, int *hourDay, int *minuteDay, int *hourDaySP, int *minuteDaySP, int day, int month, int year, double *VMean, double *IMean, double *SMean, double *TMean, double VSPMean, double *PMean, double PSPMean){

    char message[10000] = "\0";
    char res[500];
    sprintf(res, "AT+IOTPUBLISH=\"Cameskin\",1,\"{'batteries':[");
    strcat(message, res);
    for(int i = 0; i < nbBatteries; i++) {
      int n = i*nbBatteries;
      sprintf(res, "{'id':%d, 'S':[%0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e],", i, SH[n+0], SH[n+1], SH[n+2], SH[n+3], SH[n+4], SMean[i]);
      strcat(message, res);
      sprintf(res, "'V':[%0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e],", VH[0], VH[1], VH[2], VH[3], VH[4], VMean[i]);
      strcat(message, res);
      sprintf(res, "'I':[%0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e],", IH[0], IH[1], IH[2], IH[3], IH[4], IH[5], IMean[0]);
      strcat(message, res);
      sprintf(res, "'P':[%0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e]", PH[0], PH[1], PH[2], PH[3], PH[4], PH[5], PMean[i]);
      strcat(message, res);
      if(i == 0) {
        sprintf(res, ",'H':['%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d'], 'T':%0.2e, 'D': '%d-%d-%d'},", hourDay[0], minuteDay[0], hourDay[1], minuteDay[1], hourDay[2], minuteDay[2], hourDay[3], minuteDay[3], hourDay[4], minuteDay[4], TMean[i], day, month, year);
      } else if(i == nbBatteries-1) {
        sprintf(res, ",'T':%0.2e, }", TMean[i]);
      }
      else {
        sprintf(res, ",'T':%0.2e},", TMean[i]);
      }
      strcat(message, res);
    }
    sprintf(res, "], 'solarPannels': [{'D': '%d-%d-%d', 'I': %0.2e, 'V': %0.2e, 'P': [%0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e, %0.2e], 'H':['%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d', '%d:%d']}]}\"", day, month, year, IMean[1], VSPMean, PSPH[0], PSPH[1], PSPH[2], PSPH[3], PSPH[4], PSPH[5], PSPH[6], PSPH[7], PSPH[8], hourDay[0], minuteDay[0], hourDay[1], minuteDay[1], hourDay[2], minuteDay[2], hourDay[3], minuteDay[3], hourDay[4], minuteDay[4], hourDay[5], minuteDay[5], hourDay[6], minuteDay[6], hourDay[7], minuteDay[7], hourDay[8], minuteDay[8]);
    strcat(message, res);
    Serial.println("message :");
    Serial.println(message);

    //Serial1.println(message);
}

void convertMessage(String input, bool *on, int *mode, double *X, double *Z, 
              double *SOCOCV, double *dSOCOCV, double *P_x, double *P_z, double *Q_x,
              double *Q_z, double *alpha_x, double *betha_x, double *alpha_z, double *betha_z, double *Qn_rated, double *current_rated, double *voltage_rated) {

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  StaticJsonDocument<3500> doc;
  //input.replace("'", "\"");
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, String(input));
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
    Serial.println(error.f_str());
  }

  String option = doc["option"];
  Serial.print("option = ");Serial.println(option);

  if(option =="A") {
      String val = doc["value"];
      if(val == "on") { *on = true; }
      if(val == "pause") { *on = false; }
  } else if(option == "B") {
      messageLCD("reset all");
      delay(500);
      REQUEST_EXTERNAL_RESET;
  } else if(option == "C") {
    if(*on != true) {
      *mode = doc["value"];
    }
  } else if(option == "D") {
      X[0] = doc["SOC_init"];
      alpha_x[0] = doc["alpha_x"];
      betha_x[0] = doc["betha_x"];
      alpha_z[0] = doc["alpha_z"];
      betha_z[0] = doc["betha_z"];
      *Qn_rated = doc["Qn_rated"]; *Qn_rated = *Qn_rated * 3600.0;
      *voltage_rated = doc["voltage_rated"];
      *current_rated = doc["current_rated"];

      int R0 = doc["R0"]; int R1 = doc["R1"]; int R2 = doc["R2"]; int C1 = doc["C1"]; int C2 = doc["C2"]; 
      Z[0] = R0; Z[1] = R1; Z[2] = R2;
      Z[3] = R1 * C1;
      Z[4] = R2 * C2;
      
      for (int i = 0; i < 5; i++) {
        SOCOCV[i] = doc["SOCOCV"][i];
        if(i < 4) {
          dSOCOCV[i] = doc["dSOCOCV"][i];
        }
      }
    
      for (int i = 0; i < 4; i++) {
        if(i%8 == 0) {
          P_x[i] = doc["P_x"];
          Q_x[i] = doc["Q_x"];
        } else {
          P_x[i] = 0.0;
          Q_x[i] = 0.0;
        }
      }
    
      for (int i = 0; i < 9; i++) {
        if(i%6 == 0) {
          P_z[i] = doc["P_z"];
          Q_z[i] = doc["Q_z"];
        } else {
          P_z[i] = 0.0;
          Q_z[i] = 0.0;
        }
      }
  }
}
