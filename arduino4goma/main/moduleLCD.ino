void printLCD(int option, double voltage, double current, double SOC, bool on) {
    char res[150];
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
        lcd.print(current);
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
    }
}
