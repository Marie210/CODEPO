void printLCD(int option, double voltage, int Vnb, int Inb, double current, double SOC, bool on, int mode, double temperature) {
    lcd.clear();
    lcd.setCursor(0,0);
    Serial.print("option = "); Serial.println(option);
    Serial.print("voltage = "); Serial.println(voltage);
    
    if(option == 0) {
        lcd.print("Voltage :");
        lcd.setCursor(0, 1);
        char message[15];
        sprintf(message, "BAT%d : %f",Vnb, voltage);
        lcd.print(message);
    } else if(option == 1) {
        lcd.print("Current :");
        lcd.setCursor(0, 1);
        char message[16];
        if(Inb == 0) {
          sprintf(message, "BAT : %f", current);
        } else {
          sprintf(message, "SP : %f", current);
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
        sprintf(message, "BAT%d : %f",Vnb, SOC);
        lcd.print(message);
    } else if(option == 4) {
        lcd.print("State :");
        if(on) { 
          lcd.setCursor(0, 1);
          lcd.print("ON"); 
        } else { 
          lcd.setCursor(0, 1);
          lcd.print("OFF"); 
        }
    } else if(option == 5) {
        lcd.print("Temperature :");
        lcd.setCursor(0, 1);
        char message[15];
        sprintf(message, "BAT%d : %f",Vnb, temperature);
        lcd.print(message);
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


void updateLCD(int *affichage, bool *on, int *mode, double *V, int *Vnb, int *Inb, int nbBatteries, double *I, double *X, double *T, int upPin, int downPin, int rightPin, int leftPin, bool *stateUP, bool *stateDOWN, bool *stateRIGHT, bool *stateLEFT) {
      bool BUTTON_UP = digitalRead(upPin);
      bool BUTTON_DOWN = digitalRead(downPin);
      bool BUTTON_RIGHT = digitalRead(rightPin);
      bool BUTTON_LEFT = digitalRead(leftPin);
    
      if (BUTTON_UP != *stateUP) {
        Serial.println("UP");
        *stateUP = BUTTON_UP;
        if(*affichage < 5) {
          *affichage += 1;
          printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*3], *on, *mode, T[*Vnb]);
        }
      }
      if (BUTTON_DOWN != *stateDOWN) {
        *stateDOWN = BUTTON_DOWN;
        Serial.print("DOWN");
        if(*affichage > 0) {
          *affichage -= 1;
          printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*3], *on, *mode, T[*Vnb]);
        }
      }
      if(BUTTON_RIGHT != *stateRIGHT) {
        *stateRIGHT = BUTTON_RIGHT;
        Serial.println("RIGHT");
        if(*affichage == 2) {
            *mode += 1;
            if(*mode > 2) {
              *mode = 0;
            }
            printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*3], *on, *mode, T[*Vnb]);
        } else if(*affichage == 4) {
            *on = !*on;
            printLCD(*affichage, V[*Vnb], *Vnb, *Inb, I[*Inb], X[(*Vnb)*3], *on, *mode, T[*Vnb]);
        } else if(*affichage == 0 || *affichage == 5 || *affichage == 3) {
          *Vnb += 1;
          if(*Vnb > nbBatteries-1) {
              *Vnb = 0;
            }
        } else if(*affichage = 1) {
          *Inb += 1;
          if(*Inb > 1) {
            *Inb = 0;
          }
        }
      }
      if(BUTTON_LEFT != *stateLEFT) {
        *stateLEFT = BUTTON_LEFT;
        Serial.println("LEFT");
        messageLCD("reset all");
        delay(500);
        REQUEST_EXTERNAL_RESET;
      }
      Serial.print("stateDOWN3 : "); Serial.println(*stateDOWN);
}
