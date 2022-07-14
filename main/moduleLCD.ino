void printLCD(int option, double voltage, double current, double SOC, bool on, int mode, double temperature) {
    lcd.clear();
    lcd.setCursor(0,0);
    
    if(option == 0) {
        lcd.print("Voltage :");
        lcd.setCursor(0, 1);
        lcd.print(voltage);
    } else if(option == 1) {
        lcd.print("Current :");
        lcd.setCursor(0, 1);
        lcd.print(current);
    } else if(option == 2) {
        lcd.print("Mode :");
        lcd.setCursor(0, 1);
        lcd.print(mode);
    } else if(option == 3) {
        lcd.print("SOC :");
        lcd.setCursor(0, 1);
        lcd.print(SOC);
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
        lcd.print(temperature);
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

bool activateClick(int *option, int upPin, int downPin, int rightPin, int leftPin) {
    int BUTTON_UP = digitalRead(upPin);
    int BUTTON_DOWN = digitalRead(downPin);
    int BUTTON_RIGHT = digitalRead(rightPin);
    int BUTTON_LEFT = digitalRead(leftPin);

    bool res = false;
    if(BUTTON_UP || BUTTON_DOWN || BUTTON_RIGHT) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Activate click ?");
    }
    if (BUTTON_UP) {
      if(*option < 1) {
        *option += 1;
      }
      lcd.setCursor(0, 1);
      lcd.print("Yes");
    }
    if (BUTTON_DOWN) {
      if(*option > 0) {
        *option -= 1;
      }
      lcd.setCursor(0, 1);
      lcd.print("No");
    }
    if(BUTTON_RIGHT) {
      res = true;
    }
    if(BUTTON_LEFT) {
      messageLCD("reset all");
      delay(500);
      REQUEST_EXTERNAL_RESET;
    }
    return res;
}


void updateLCD(int *affichage, bool *on, int *mode, double Vt_Actual, double U, double X, double temperature, int upPin, int downPin, int rightPin, int leftPin) {
      int BUTTON_UP = digitalRead(upPin);
      int BUTTON_DOWN = digitalRead(downPin);
      int BUTTON_RIGHT = digitalRead(rightPin);
      int BUTTON_LEFT = digitalRead(leftPin);
    
      if (BUTTON_UP) {
        if(*affichage < 5) {
          *affichage += 1;
          printLCD(*affichage, Vt_Actual, U, X, *on, *mode, temperature);
        }
      }
      if (BUTTON_DOWN) {
        if(*affichage > 0) {
          *affichage -= 1;
          printLCD(*affichage, Vt_Actual, U, X, *on, *mode, temperature);
        }
      }
      if(BUTTON_RIGHT) {
        if(*affichage == 2) {
            *mode += 1;
            if(*mode > 2) {
              *mode = 0;
            }
            printLCD(*affichage, Vt_Actual, U, X, *on, *mode, temperature);
        } else if(*affichage == 4) {
            *on = !*on;
            printLCD(*affichage, Vt_Actual, U, X, *on, *mode, temperature);
        }
      }
      if(BUTTON_LEFT) {
        messageLCD("reset all");
        delay(500);
        REQUEST_EXTERNAL_RESET;
      }
}
