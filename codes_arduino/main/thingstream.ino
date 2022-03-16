int initThingstream(int flag_init) {
  Serial.println("INITIALISATION");
  if (flag_init == 0 ){
    Serial1.println("AT+IOTDEBUG=0");
    
    if(checkReception() == 1) { flag_init = 1; }
    Serial.println(flag_init);
  }
  else if(flag_init == 1){
    Serial1.println("AT+IOTCREATE");   
    
    if(checkReception() == 1) { flag_init = 2; }
    Serial.println(flag_init);
  }
  else if(flag_init == 2){
    Serial1.println("AT+IOTCONNECT=true"); 
     
    if(checkReception() == 1) { flag_init = 3; }
    Serial.println(flag_init);
  }
  else if(flag_init == 3) {
    Serial1.println("AT+IOTSUBSCRIBE=\"TEST1\",1");
    delay(5000);
    
    if(checkReception() == 1) { flag_init = 4; }
    Serial.println(flag_init);
  }
  return flag_init;
}

int checkReception() {
  bool check = true;
  unsigned long timeInit = millis()*0.001;
  char message[150];
  int count = 0;
  
  while(check) {

    Serial.println(millis()*0.001 - timeInit);  
    if(millis()*0.001 - timeInit > 20.0) { 
      Serial.println("To Long Time");  
      return 0;
     }

     if (Serial1.available ()){
        char st = Serial1.read();
        count += 1; 
        message[count] = st;  
        message[count+1] = '\0';  
                
        if(message[count] == '\n'){
          Serial.println(String(message));  
          if (analyse(message) ==  1) {
            Serial.println("SUCCESS");  
            return 1;
          } else {
            Serial.println("FAILURE");
            return 0;
          }
        }
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

int publish(double Vt_Actual, double U, double X[3], double Z[5], double error){

    char message[150];
    sprintf(message, "AT+IOTPUBLISH=\"TEST1\",1,\"{'tension': %d, 'courant': %d, 'Error': %d, 'SOC': %d}\"", Vt_Actual, U, error, X[0]);
    
    // envoie du message vers le Thingstream click
    Serial1.println(message);
    
    // verification si le message a bien été réceptionné ou non
    int flag = checkReception();
    if(flag == 1) {
      Serial.println("SUCCSESS -- envoie du message :");
    } else {
      Serial.println("FAIL -- envoie du message :");
    }
}
