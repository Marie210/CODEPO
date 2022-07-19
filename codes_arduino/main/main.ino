/* --- code pour reset la due à distance --- */
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
#include <LiquidCrystal_I2C.h>

/* ----- PIN DECLARATION ----- */
const int CS_PIN = 4;
/* -- BUTTONS --*/
const int upPin = 9;
const int downPin = 10;
const int rightPin = 11;
const int leftPin = 12;
/* -- RELAY -- */
const int relayPin = 2;
/* -- THINGSTREAM -- */
const int resetThingstreamPin = 3;
/* -- MEASURES -- */
const byte VPin = A0;
const byte pinI = A4;
const byte VccIPin = A8;
const byte VccPin = A2;
const byte pinISP = A5;
const byte VccIPinSP = A3;
const byte TPin = A6;
/* -- MULTIPLEXEUR -- */
const byte PIN_ENABLE = 8;
const byte PIN_ADDR_A = 5;
const byte PIN_ADDR_B = 6;
const byte PIN_ADDR_C = 7;

/* ----- VARIABLES DECLARATION ----- */
/* -- LCD -- */
LiquidCrystal_I2C lcd(0x27,16,2);  // met l'adresse du LCD sur 0x27
unsigned long lcdTime = 0.0;
double updateLCDTime = 300;
/* -- SD --*/
File file;
const char *filename = "/data.txt"; 
/* -- MEASURES -- */
const int nbBatteries = 6; // nombre de batteries => de 1 à 6 + 1 pour panneux solaires
const int nbCurrent = 2;
double I[nbCurrent] = {0.0, 0.0};
double V[nbBatteries] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double T[nbBatteries] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double VSP = 0.0;
int Vnb = 0;
int Inb = 0;
double Valim = 0.0;
double RV = 32300;
double pot[nbBatteries] = {10000, 10000, 10000, 10000, 10000, 10000}; 
double RVcc1 = 500000; 
double RVcc2 = 500000;
double RT = 4700;
double RSP1 = 1000000;
double RSP2 = 20400;
int numSamples = 100;
double mvPerI = 0.03125;
/* -- FILTRE KALEMAN-- */
double X[3*nbBatteries];
double Z[5*nbBatteries];
double SOCOCV[12];
double dSOCOCV[11];
double P_x[9*nbBatteries];
double P_z[25*nbBatteries];
double Q_x[9*nbBatteries]; 
double Q_z[25*nbBatteries];
double R_x[nbBatteries]; 
double R_z[nbBatteries];
double DeltaT; 
double Qn_rated;
double voltage_rated;
double current_rated;
double error = 0.0;
unsigned long kalmanTime = 0.0;
double SoC_coulomb[nbBatteries] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
/* -- HPPC -- */
bool onHPPC = false;
int HPPC_time = 10;
int HPPC_timeOn = 30;
int HPPC_timeOff = 60;
/* -- BUTTON -- */
unsigned long buttonTime = 0.0;
bool stateUP = 0;
bool stateDOWN = 0; 
bool stateRIGHT = 0;
bool stateLEFT = 0;
bool test = 0;
double updateButtonTime = 200;
/* -- DIVERS -- */
int mode = 0, oldMode = 0; // default = 0
int counter = 0;
bool on = true; // default = false
double seconds = 0.0;
double sec = 0.0;
int affichage = 0;
unsigned long previous_millis = 0;  
int  flag_init = -1; // flag utilisé pour l'initialisation de l'arduino et du click (default = -1)
int flag_publish = 1;  // si sur 1 publish activé si sur 0 publish désactivé 
unsigned long previous_millis_publish = 0.0;
bool start = false;
bool initClick = false;


void setup() {
  /*--- Initialisation des communications serial ---*/
  Serial.begin(115200);
  Serial1.begin(115200);                  

  /*--- Initialisation de la carte SD ---*/
  initializeSD();

  /*--- Initialisation LCD ---*/
  
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(0,0);   
  lcd.print("Starting...");
  messageLCD2Lines("Activate click ?", "No");

  /*--- Initialisation des pins ---*/
  pinMode(relayPin, OUTPUT);
  pinMode(resetThingstreamPin, OUTPUT);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(leftPin, INPUT_PULLUP);
  digitalWrite(relayPin,HIGH);

  /* -- initialisation de l'etat des boutons -- */
  stateUP = digitalRead(upPin);
  stateDOWN = digitalRead(downPin);
  stateRIGHT = digitalRead(rightPin);
  stateLEFT = digitalRead(leftPin);

  /* --- Initialisation du multiplexeur --- */
  // Output à 000
  pinMode(PIN_ADDR_A, OUTPUT);
  pinMode(PIN_ADDR_B, OUTPUT); 
  pinMode(PIN_ADDR_C, OUTPUT); 
  digitalWrite(PIN_ADDR_A, LOW);
  digitalWrite(PIN_ADDR_B, LOW);
  digitalWrite(PIN_ADDR_C, LOW);
  // Active le multiplexeur
  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, LOW);
}

void loop() {

  if(flag_init == -1) {
      if(millis() - buttonTime > 100) {
        buttonTime = millis();
          if(activateClick(&affichage, upPin, downPin, rightPin, leftPin, &stateUP, &stateDOWN, &stateRIGHT, &stateLEFT)) {
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
        char input[] = "{'C1': 1.704510181209152, 'C2': 1.528807876468525, 'R0': 0.045060113042053, 'R1': 0.022227266870883, 'R2': 0.041772924359798, 'SOCOCV': [-5899.08970208633, 33676.8099461629, -83441.9838542838, 117439.916788238, -103158.271786075, 58466.9571460381, -21320.9039215880, 4810.53130689387, -602.846517986747, 26.7902085666153, 3.13908872646908, 5.40144738302999], 'dSOCOCV': [-64889.9867229497, 336768.099461629, -750977.854688554, 939519.334305904, -722107.902502523, 350801.742876229, -106604.519607940, 19242.1252275755, -1808.53955396024, 53.5804171332306, 3.13908872646908], 'SOC_init': 1.0, 'P_x': 5e-7, 'P_z': 5e-5, 'Q_x': 1e-8, 'Q_z': 1e-8, 'R_x': 1, 'R_z': 10, 'Qn_rated': 1.3, 'voltage_rated': 2, 'current_rated': 1.3}";
        initialisation(input, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, R_x, R_z, &Qn_rated, &voltage_rated, &current_rated);
        duplicate(X, Z, P_x, P_z, Q_x, Q_z, R_x, R_z, nbBatteries);
        flag_init = 5;
  } 
  else if(flag_init == 5) {
      if(mode == 0 && on) {
        /* --- Measure VOLTAGE + CURRENT + TEMP ---*/
        takeMeasures(V, I, T, nbBatteries, nbCurrent, PIN_ADDR_A, PIN_ADDR_B, PIN_ADDR_C, RV, pot, numSamples, VPin, pinI, VccIPin, pinISP, VccIPinSP, mvPerI, TPin, VccPin, RT, RVcc1, RVcc2);
    
        /* --- AFFICHAGE ---*/
        sec = millis()*0.001;
    
        DeltaT = millis()*0.001 - kalmanTime;
        kalmanTime = millis()*0.001;
        DeltaT = 0.5;
        
        /* --- COULOMB Counting ---*/
        for(int i = 0; i < nbBatteries; i++) {
          SoC_coulomb[i] -= (I[0] * DeltaT) / Qn_rated; // Estimated SoC in %
        }
      
        /* --- KALMAN FILTER ---*/
        kalmanFilter(I[0], X, Z, SOCOCV, dSOCOCV, V, P_x, P_z, Q_x, Q_z, R_x, R_z, DeltaT, Qn_rated, nbBatteries);
                
        //String message = createData(V[0], U, X, Z, error);
        if (millis()  >=  previous_millis + 60000 && flag_publish == 1 && initClick == true) {
          previous_millis =  millis();
          Serial.print("publish");
          publish(V[0], I[0], X, Z, error, T[0]);
        }
      
    } else if(mode == 1 && on) {

        V[0] = measureVoltage(RV, pot[0], numSamples, VPin);
        I[0] = measureCurrent(pinI, VccIPin, mvPerI, numSamples);
        T[0] = mesureTemperature(TPin, VccPin, RT, RVcc1, RVcc2);
        
        /* ---- save data to SD card ---- */
        printSd(V[0], I[0], sec, T[0]);
        
        sec = millis()*0.001;
        if(sec-previous_millis > HPPC_time) {
          previous_millis = sec;
          onHPPC = !onHPPC;
          if(onHPPC) {
            digitalWrite(relayPin,HIGH);
            HPPC_time = HPPC_timeOff;
          } else {
            digitalWrite(relayPin,LOW);
            HPPC_time = HPPC_timeOn;
          }
        }
    } else if(mode == 2 && on) {
        /* ---- MEASURES => ALIMENTATION - VOLTAGE - CURRENT - TEMPERATURE - TIME ---- */
        V[0] = measureVoltage(RV, pot[0], numSamples, VPin);
        I[0] = measureCurrent(pinI, VccIPin, mvPerI, numSamples);
        T[0] = mesureTemperature(TPin, VccPin, RT, RVcc1, RVcc2);
        sec = millis()*0.001;
        /* ---- save data to SD card ---- */
        //printSd(V[0], I[0], sec, T[0]);
    }

    if(millis() - buttonTime > updateButtonTime) {
      buttonTime = millis();
      updateLCD(&affichage, &on, &mode, V, &Vnb, &Inb, nbBatteries, I, X, T, upPin, downPin, rightPin, leftPin, &stateUP, &test, &stateRIGHT, &stateLEFT);
    }

    if(millis() - lcdTime > updateLCDTime) {
      lcdTime = millis();
      printLCD(affichage, V[Vnb], Vnb, Inb, I[Inb], X[Vnb*3], on, mode, T[Vnb]);
    }

    if(oldMode != mode) {
      digitalWrite(relayPin,HIGH);
      oldMode = mode;
    }
    
  } else {
    initThingstream(&flag_init, resetThingstreamPin);
  }
  
  
   String receivedMessage;
   if(readCommand(&receivedMessage)) {
      convertMessage(receivedMessage, &on, &mode, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, R_x, R_z, &Qn_rated, &current_rated, &voltage_rated);
      duplicate(X, Z, P_x, P_z, Q_x, Q_z, R_x, R_z, nbBatteries);
   }

}
