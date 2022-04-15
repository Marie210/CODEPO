/* --- code pour reset la due depuis le software --- */
#define SYSRESETREQ    (1<<2)
#define VECTKEY        (0x05fa0000UL)
#define VECTKEY_MASK   (0x0000ffffUL)
#define AIRCR          (*(uint32_t*)0xe000ed0cUL) // fixed arch-defined address
#define REQUEST_EXTERNAL_RESET (AIRCR=(AIRCR&VECTKEY_MASK)|VECTKEY|SYSRESETREQ)
#define WDT_KEY (0xA5)
#define WHITE 0x7
/* ------------------------------------------------- */
#include <extendedKalmanFilter.h> 
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>


Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
/* ----- PIN DECLARATION ----- */
int CS_PIN = 4;

/* ----- VARIABLEs DECLARATION ----- */
File file;
int mode = 0;
int counter = 0;
bool on = false;
bool onHPPC = false;
bool check = true;
int HPPC_time = 10;
const char *filename = "/data.txt"; 
double parameters[5]; // = [C1, C2, R0, R1, R2]
double U = 0;
double X[3];
double Z[5];
double SOCOCV[12];
double dSOCOCV[11];
double Vt_Actual = 0.0;
double Vt_alim = 0.0;
double P_x[9];
double P_z[25];
double Q_x[9]; 
double Q_z[25];
double R_x;
double R_z;
double DeltaT; 
double Qn_rated;
double voltage_rated;
double current_rated;
double error = 0.0;
double cat = 8; 
double Q_consumed = 1.0;
double SoC_coulomb;
double seconds = 0.0;
double temperature;
double sec = 0.0;
int affichage = 0;
unsigned long previous_millis = 0;  
int  flag_init = -1; // flag utilisé pour l'initialisation de l'arduino et du click 
int flag_publish = 1;  // si sur 1 publish activé si sur 0 publish désactivé 
unsigned long previous_millis_publish = 0.0;
unsigned long buttonTime = 0.0;
unsigned long lcdTime = 0.0;
unsigned long kalmanTime = 0.0;
bool start = false;
bool initClick = false;


void watchdogSetup(void) {/*** watchdogDisable (); ***/}
void WDT_Handler(void)
{
  
  WDT->WDT_SR; // Clear status register
  printf("help! in WDT\n");
}


void setup() {
  /*--- Initialisation des communications serial ---*/
  Serial.begin(115200);
  Serial1.begin(115200);

  /*--- Enable watchdog ---*/
  
  WDT->WDT_MR = WDT_MR_WDD(0xFFF)
                | WDT_MR_WDRPROC
                | WDT_MR_WDRSTEN
                | WDT_MR_WDV(256 * 20); // Watchdog triggers a reset after 2 seconds if underflow
                                          // 2 seconds equal 84000000 * 2 = 168000000 clock cycles                     

  /*--- Initialisation de la carte SD ---*/
  initializeSD();

  /*--- Initialisation LCD ---*/
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  lcd.print("Starting...");
  messageLCD2Lines("Activate click ?", "No");

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2,HIGH);
  
}

void loop() {

  while(true) {
    delay(10);
    Serial.println(millis());
  }

  if(flag_init == -1) {
      if(millis() - buttonTime > 100) {
        buttonTime = millis();
        if(activateClick(&affichage)) {
          if(affichage == 0) {
            flag_init = 4;
            initClick = false;
          } else {
            flag_init = 0;
            initClick = true;
          }
        }  
      }
  }
  else if(flag_init == 4) {
        char input[] = "{'C1': 1.704510181209152, 'C2': 1.528807876468525, 'R0': 0.045060113042053, 'R1': 0.022227266870883, 'R2': 0.041772924359798, 'SOCOCV': [-5899.08970208633, 33676.8099461629, -83441.9838542838, 117439.916788238, -103158.271786075, 58466.9571460381, -21320.9039215880, 4810.53130689387, -602.846517986747, 26.7902085666153, 3.13908872646908, 5.40144738302999], 'dSOCOCV': [-64889.9867229497, 336768.099461629, -750977.854688554, 939519.334305904, -722107.902502523, 350801.742876229, -106604.519607940, 19242.1252275755, -1808.53955396024, 53.5804171332306, 3.13908872646908], 'SOC_init': 0.5, 'P_x': 5e-7, 'P_z': 5e-5, 'Q_x': 1e-8, 'Q_z': 1e-8, 'R_x': 1, 'R_z': 10, 'Qn_rated': 1.3, 'voltage_rated': 2, 'current_rated': 1.3}";
        initialisation(input, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, &R_x, &R_z, &Qn_rated, &voltage_rated, &current_rated);
        flag_init = 5;
  } 
  else if(flag_init == 5) {
      if(mode == 0 && on) {
        
        /* --- Measure VOLTAGE + CURRENT ---*/
        measureVoltage(&Vt_alim, &U, &Vt_Actual);
        temperature = mesureTemperature();
    
        /* --- AFFICHAGE ---*/
        
        sec = millis()*0.001;

        /*
        Serial.print(sec, 5); Serial.print("  Ti;  ");
        Serial.print(temperature, 5); Serial.print("  Te;  ");
        Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
        Serial.print(Vt_alim, 5); Serial.print("  alim;  ");
        Serial.print(error, 5); Serial.print("  E;  ");
        Serial.print(U, 5); Serial.print("  U;  ");
        Serial.print(X[0], 5); Serial.print("  S;  ");
        Serial.print(SoC_coulomb, 5); Serial.print("  C;  ");
        Serial.print("\n");
        */      
    
        if(true) {
          DeltaT = millis()*0.001 - kalmanTime;
          if(!start) {
            DeltaT = 1.0;
            start = true;
          }
          kalmanTime = millis()*0.001;

          /* --- COULOMB Counting ---*/
          Q_consumed += U * DeltaT; // current * time; => Q_consumed [A.s]
          SoC_coulomb = 0.0 - (Q_consumed) / Qn_rated; // Estimated SoC in %
      
          /* --- KALMAN FILTER ---*/
          error = extendedKalmanFilter(U, X, Z, SOCOCV, dSOCOCV, Vt_Actual, P_x, P_z, Q_x, Q_z, R_x, R_z, DeltaT, Qn_rated);
          Serial.print("--- DeltaT = "); Serial.println(DeltaT);
        }
                
        //String message = createData(Vt_Actual, U, X, Z, error);
        if (millis()  >=  previous_millis + 60000 && flag_publish == 1 && initClick == true) {
          previous_millis =  millis();
          Serial.print("publish");
          publish(Vt_Actual, U, X, Z, error, temperature);
        }
      
    } else if(mode == 1 && check == true && on) {
      
        measureVoltage(&Vt_alim, &U, &Vt_Actual);
        temperature = mesureTemperature();
        
        Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
        Serial.print(U, 5); Serial.print("  U;  ");
        Serial.print(sec, 5); Serial.print("  Ti;  ");
        Serial.print("\n");
        
        /* ---- save data to SD card ---- */
        printSd(Vt_Actual, U, sec, temperature);
        
        sec = millis()*0.001;
        if(sec-previous_millis > HPPC_time) {
          previous_millis = sec;
          onHPPC = !onHPPC;
          if(onHPPC) {
            digitalWrite(2,HIGH);
            HPPC_time = 10;
          } else {
            digitalWrite(2,LOW);
            HPPC_time = 5;
          }
        }
    } else if(mode == 2 && on) {
        /* ---- MEASURES => ALIMENTATION - VOLTAGE - CURRENT - TEMPERATURE - TIME ---- */
        measureVoltage(&Vt_alim, &U, &Vt_Actual);
        temperature = mesureTemperature();
        sec = millis()*0.001;
        /* ---- save data to SD card ---- */
        printSd(Vt_Actual, U, sec, temperature);
    }

    if(millis() - buttonTime > 200) {
      buttonTime = millis();
      updateLCD(&affichage, &on, &mode, Vt_Actual, U, X[0], temperature);
    }

    if(millis() - lcdTime > 300 && flag_init == 5) {
      lcdTime = millis();
      printLCD(affichage, Vt_Actual, U, X[0], on, mode, temperature);
    }

    if(!on){
      start = false;
    }
    
  } else {
    initThingstream(&flag_init);
  }
  
  
   String receivedMessage;
   if(readCommand(&receivedMessage)) {
      convertMessage(receivedMessage, &on, &mode, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, &R_x, &R_z, &Qn_rated, &current_rated, &voltage_rated);
   }

    WDT->WDT_CR = WDT_CR_KEY(WDT_KEY)
                  | WDT_CR_WDRSTT;
}
