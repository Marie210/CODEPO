void initThingstream(int *flag_init) {
  if (*flag_init == 0 ){
    // reset the Thingstream click
    digitalWrite(3, LOW);
    delay(1200);
    digitalWrite(3, HIGH);
    Serial.println("DEBUG");
    Serial1.println("AT+IOTDEBUG=0");
    
    if(checkReception() == 1) { *flag_init = 1; }
  }
  else if(*flag_init == 1){
    Serial.println("CREATE");
    Serial1.println("AT+IOTCREATE");  
    
    if(checkReception() == 1) { *flag_init = 2; }
  }
  else if(*flag_init == 2){
    Serial.println("CONNECT");
    Serial1.println("AT+IOTCONNECT=true"); 
     
    if(checkReception() == 1) { *flag_init = 3; }
  }
  else if(*flag_init == 3) {
    Serial.println("SUBSCRIBE");
    Serial1.println("AT+IOTSUBSCRIBE=\"TEST1\",1");
    
    if(checkReception() == 1) { *flag_init = 4; }
  }
}

int checkReception() {
  bool check = true;
  unsigned long timeInit = millis()*0.001;
  char message[150];
  int count = 0;
  
  while(check) {
    
    if(millis()*0.001 - timeInit > 20.0) { 
      Serial.println("To Long Time");
      check = false;  
      return 0;
     }

     if (Serial1.available()){
        char st = Serial1.read();
        message[count] = st;  
        message[count+1] = '\0';  
        count += 1; 
                
        if(message[count-1] == '\n'){
          Serial.print(String(message));  
          if (analyse(message) ==  1) {
            check = false;
            return 1;
          } else {
            check = false;
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

int publish(double Vt_Actual, double U, double X[3], double Z[5], double error, double temperature){

    char message[150];
    sprintf(message, "AT+IOTPUBLISH=\"TEST1\",1,\"{'tension': %f, 'courant': %f, 'Error': %f, 'SOC': %f, 'temperature' : %f}\"", Vt_Actual, U, error, X[0], temperature);

    
    // envoie du message vers le Thingstream click
    Serial1.println(message);
    
    // verification si le message a bien été réceptionné ou non
    int flag = checkReception();
    if(flag == 1) {
      Serial.println("SUCCSESS -- envoie du message :");
      Serial.println(message);
    } else {
      Serial.println("FAIL -- envoie du message :");
      Serial.println(message);
    }
}

void(* resetFunc) (void) = 0;

void convertMessage(String input, bool *on, int *mode, double X[3], double Z[5], 
              double SOCOCV[12], double dSOCOCV[11], double P_x[9], double P_z[25], double Q_x[9],
              double Q_z[25], double *R_x, double *R_z, double *Qn_rated, double *current_rated, double *voltage_rated) {

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
      resetFunc();
  } else if(option == "C") {
    if(*on != true) {
      *mode = doc["value"];
    }
  } else if(option == "D") {
      X[0] = doc["SOC_init"];
      *R_x = doc["R_x"];
      *R_z = doc["R_z"];
      *Qn_rated = doc["Qn_rated"];
      *voltage_rated = doc["voltage_rated"];
      *current_rated = doc["current_rated"];

      int R0 = doc["R0"]; int R1 = doc["R1"]; int R2 = doc["R2"]; int C1 = doc["C1"]; int C2 = doc["C2"]; 
      Z[0] = R0; Z[1] = R1; Z[2] = R2;
      Z[3] = R1 * C1;
      Z[4] = R2 * C2;
      
      for (int i = 0; i < 12; i++) {
        SOCOCV[i] = doc["SOCOCV"][i];
      }
    
      for (int i = 0; i < 11; i++) {
        if(i%8 == 0) {
          P_x[i] = doc["P_x"];
          Q_x[i] = doc["Q_x"];
        } else {
          P_x[i] = 0.0;
          Q_x[i] = 0.0;
        }
        dSOCOCV[i] = doc["dSOCOCV"][i];
      }
    
      for (int i = 0; i < 25; i++) {
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
