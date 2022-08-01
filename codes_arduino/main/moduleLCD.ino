void printLCD(int option, double voltage, int Vnb, int Inb, double current, double SOC, bool on, int mode, bool SDon, double temperature, int d ,int mo, int ye,  int h,  int mi,  int s) {
    lcd.clear();
    lcd.setCursor(0,0);
    
    if(option == 0) {
        lcd.print("Voltage :");
        lcd.setCursor(0, 1);
        char message[15];
        sprintf(message, "BAT%d : %0.2f",Vnb, voltage);
        lcd.print(message);
    } else if(option == 1) {
        lcd.print("Current :");
        lcd.setCursor(0, 1);
        char message[16];
        if(Inb == 0) {
          sprintf(message, "BAT : %0.2f", current);
        } else {
          sprintf(message, "SP : %0.2f", current);
        }
        lcd.print(message);
    } else if(option == 2) {
        lcd.print("Mode :");
        lcd.setCursor(0, 1);
        lcd.print(mode);
    } else if(option == 3) {
        lcd.print("SOC :");
        lcd.setCursor(0, 1);
        char message[15];
        sprintf(message, "BAT%d : %0.2f",Vnb, SOC);
        lcd.print(message);
    } else if(option == 4) {
        lcd.print("State :");
        lcd.setCursor(0, 1);
        if(on) { 
          lcd.print("ON"); 
        } else { 
          lcd.print("OFF"); 
        }
    } else if(option == 5) {
        lcd.print("Temperature :");
        lcd.setCursor(0, 1);
        char message[15];
        sprintf(message, "BAT%d : %0.2f",Vnb, temperature);
        lcd.print(message);
    }
    else if (option == 6){
      lcd.print("Date :") ;  
      lcd.setCursor(0,1) ; 
      char message[25];  
      sprintf(message, "%d-%d-%d", d, mo, ye);  
      lcd.print(message);
      
    }
    else if (option == 7) {
      lcd.print("Heure :") ;  
      lcd.setCursor(0,1) ; 
      char message[15];  
      sprintf(message, "%d:%d:%d", h, mi, s);  
      lcd.print(message);
    }
    else if (option == 8) {
      lcd.print("SAVE ON SD CARD :");  
      lcd.setCursor(0, 1);
      if(SDon) { 
          lcd.print("YESS"); 
        } else { 
          lcd.print("NO"); 
        }
    }
}

void messageLCD(String message) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(message);
}

void messageLCD2Lines(String message1, String message2) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(message1);
    lcd.setCursor(0, 1);
    lcd.print(message2); 
}

bool activateClick(int *option, int upPin, int downPin, int rightPin, int leftPin, bool *stateUP, bool *stateDOWN, bool *stateRIGHT, bool *stateLEFT) {
    bool BUTTON_UP = digitalRead(upPin);
    bool BUTTON_DOWN = digitalRead(downPin);
    bool BUTTON_RIGHT = digitalRead(rightPin);
    bool BUTTON_LEFT = digitalRead(leftPin);

    bool res = false;
    if(BUTTON_UP != *stateUP || BUTTON_DOWN != *stateDOWN || BUTTON_RIGHT != *stateRIGHT) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Activate click ?");
    }
    if (BUTTON_UP != *stateUP) {
      *stateUP = BUTTON_UP;
      if(*option < 1) {
        *option += 1;
      }
      lcd.setCursor(0, 1);
      lcd.print("Yes");
    }
    if (BUTTON_DOWN != *stateDOWN) {
      *stateDOWN = BUTTON_DOWN;
      if(*option > 0) {
        *option -= 1;
      }
      lcd.setCursor(0, 1);
      lcd.print("No");
    }
    if(BUTTON_RIGHT != *stateRIGHT) {
      *stateRIGHT = BUTTON_RIGHT;
      res = true;
    }
    if(BUTTON_LEFT != *stateLEFT) {
      *stateLEFT = BUTTON_LEFT;
      messageLCD("reset all");
      delay(500);
      REQUEST_EXTERNAL_RESET;
    }
    return res;
}


void updateLCD(int *affichage, bool *on, int *mode, bool *SDon, double *V, int *Vnb, int *Inb, int nbBatteries, double *I, double *X, double *T, bool *stateUP, bool *stateDOWN, bool *stateRIGHT, bool *stateLEFT,int *d, int *mo, int *y, int *h, int *mi, int *s, int *count_push) {
      bool BUTTON_UP = digitalRead(upPin);
      bool BUTTON_DOWN = digitalRead(downPin);
      bool BUTTON_RIGHT = digitalRead(rightPin);
      bool BUTTON_LEFT = digitalRead(leftPin);
    
      if (BUTTON_UP != *stateUP) {
        Serial.println("---UP");
        *stateUP = BUTTON_UP;
        if(*affichage < 8 && *count_push == 0) {
          *affichage += 1;
        } else if(*affichage == 6) {
          if(*count_push == 1) {
            *d -= 1;
            if (*d < 1) { *d = 1; }
          } else if(*count_push == 2) {
            *mo -= 1;
            if (*mo < 1){ *mo = 1; }
          } else if(*count_push == 3) {
            *y -= 1;
            if(*y < 2000) { *y = 2000; }
          }
        } else if(*affichage == 7) {
          if(*count_push == 1) {
            *h -= 1;
            if (*h < 0) { *h = 0; }
          } else if(*count_push == 2) {
            *mi -= 1;
            if (*mi < 0){ *mi = 0; }
          } else if(*count_push == 3) {
            *s -= 1;
            if(*s < 0) { *s = 0; }
          }
        } 
      }
      if (BUTTON_DOWN != *stateDOWN) {
        Serial.println("---DOWN");
        *stateDOWN = BUTTON_DOWN;
        if(*affichage > 0 && *count_push == 0) {
          *affichage -= 1;
        } else if(*affichage == 6) {
          if(*count_push == 1) {
            *d += 1;
            if (*d > 31) { *d = 1; }
          } else if(*count_push == 2) {
            *mo += 1;
            if (*mo > 12){ *mo = 1; }
          } else if(*count_push == 3) {
            *y += 1;
          }
        } else if(*affichage == 7) {
          if(*count_push == 1) {
            *h += 1;
            if (*h > 24) { *d = 0; }
          } else if(*count_push == 2) {
            *mi += 1;
            if (*mi > 60){ *mi = 0; }
          } else if(*count_push == 3) {
            *s += 1;
            if (*s > 60){ *s = 0; }
          }
        }
        printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*2], *on, *mode, *SDon, T[*Vnb], *d, *mo, *y, *h, *mi, *s);
      }
      if(BUTTON_RIGHT != *stateRIGHT) {
        Serial.println("---SELECT");
        *stateRIGHT = BUTTON_RIGHT; 
        if(*affichage == 2) {
            *mode += 1;
            if(*mode > 2) {
              *mode = 0;
            }
        } else if(*affichage == 4) {
            *on = !*on;
        } else if(*affichage == 0 || *affichage == 5 || *affichage == 3) {
          *Vnb += 1;
          if(*Vnb > nbBatteries-1) {
              *Vnb = 0;
            }
        } else if(*affichage == 1) {
          *Inb += 1;
          if(*Inb > 1) {
            *Inb = 0;
          }
        } else if(*affichage == 6 || *affichage == 7) {
          *count_push += 1;
            if (*count_push == 4) { 
              *count_push = 0; 
              rtc.setTime(hours, minutes, seconds);
              rtc.setDate(day, month, year);
            }
        } else if(*affichage == 8) {
          *SDon = !*SDon;
        }
        printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*2], *on, *mode, *SDon, T[*Vnb], *d, *mo, *y, *h, *mi, *s);
      }
      if(BUTTON_LEFT != *stateLEFT) {
        Serial.println("---RESET");
        *stateLEFT = BUTTON_LEFT;
        messageLCD("reset all");
        delay(500);
        REQUEST_EXTERNAL_RESET;
      }
      printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*2], *on, *mode, *SDon, T[*Vnb], *d, *mo, *y, *h, *mi, *s);
}
