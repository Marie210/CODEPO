
int initThingstream(int flag_init) {
  Serial.println("INITIALISATION");
  if (flag_init == 0 ){
    Serial1.println("AT+IOTDEBUG=0");
    delay(5000);
    if(checkReception() == 1) { flag_init = 1; }
    Serial.println(flag_init);
    }
  if (flag_init == 1){
    Serial1.println("AT+IOTCREATE") ;
    delay(15000); 
   
    if(checkReception() == 1) { flag_init = 2; }
    Serial.println(flag_init);
  }
  if (flag_init ==2 ){
    Serial1.println("AT+IOTCONNECT=true"); 
    delay(5000);
     
    if(checkReception() == 1) { flag_init = 3; }
    Serial.println(flag_init);
  }
  if (flag_init == 3) {
   
    Serial1.println("AT+IOTSUBSCRIBE=\"TEST1\",1");
    delay(5000);
    
    if(checkReception() == 1) { flag_init = 4; }
    Serial.println(flag_init);
  }
  return flag_init;
}

int checkReception(){
  //Fonction utilisée pour vérifier le bon envoi des commandes au Click 
  int flag =  0 ; 
  if(Serial1.available()){
     String msg = Serial1.readString();
     Serial.print("reponse : ");
     Serial.println(msg);
     if(msg.indexOf("SUCCESS") > 0) {
      flag = 1;  
     }
  }
  return flag; 
}


int analyse (String st) {
  // Arret de la publication si le mot STOP est reçu par le Click
  int res = 1;  
  if(st.indexOf("STOP") > 0) {
    res = 0;  
  } else if(st.indexOf("GO") > 0) {
    res = 1;
  }
  return res ; 

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
