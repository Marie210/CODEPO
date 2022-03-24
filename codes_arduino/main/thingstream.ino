/**
ensure that the 4 initialization steps needed to connect the click and the thingstream server are done 
**/

void initThingstream(int *flag_init) {
  Serial.println("INITIALISATION");
  
  //DEBUG allows the clicks to get clear error messages
  if (*flag_init == 0 ){
    Serial.println("DEBUG");
    Serial1.println("AT+IOTDEBUG=0");
    delay(5000);
    
    if(checkReception() == 1) { *flag_init = 1; }
    Serial.println(*flag_init);
  }
  
  //CREATE create the connection between the click and the server
  else if(*flag_init == 1){
    Serial.println("CREATE");
    Serial1.println("AT+IOTCREATE");  
    delay(5000); 
    
    if(checkReception() == 1) { *flag_init = 2; }
    Serial.println(*flag_init);
  }
  
  //CONNECT is the second step of the connection between the click and the server
  else if(*flag_init == 2){
    Serial.println("CONNECT");
    Serial1.println("AT+IOTCONNECT=true"); 
    delay(5000);
     
    if(checkReception() == 1) { *flag_init = 3; }
    Serial.println(*flag_init);
  }
  
  //SUBSCRIBE to a topic allows the click to receive the message published in this topic
  else if(*flag_init == 3) {
    Serial.println("SUBSCRIBE");
    Serial1.println("AT+IOTSUBSCRIBE=\"TEST1\",1");
    delay(5000);
    
    if(checkReception() == 1) { *flag_init = 4; }
    Serial.println(*flag_init);
  }
}

/**
ensure that the message send by the click has been received by the server
**/

int checkReception() {
  bool check = true;
  unsigned long timeInit = millis()*0.001;
  char message[150];
  int count = 0;
  
  while(check) {

    /*
    double Vt_Actual = 0.0, U = 0.0;
    measureVoltage(&Vt_Actual, &U);
    Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
    Serial.print(U, 5); Serial.print("  U;  ");
    Serial.print("\n");
    */
    
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
          Serial.print(String(message));  
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

/**
Publish a message on a topic with the Thingstream click
**/
int publish(double Vt_Actual, double U, double X[3], double Z[5], double error){

    char message[150];
    sprintf(message, "AT+IOTPUBLISH=\"TEST1\",1,\"{'tension': %d, 'courant': %d, 'Error': %d, 'SOC': %d}\"", Vt_Actual, U, error, X[0]);
    
    // ask the Thingstream click to publish the message
    Serial1.println(message);
    
    // check that the message has been received
    int flag = checkReception();
    if(flag == 1) {
      Serial.println("SUCCSESS -- envoie du message :");
    } else {
      Serial.println("FAIL -- envoie du message :");
    }
}
