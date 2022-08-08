#define WDT_KEY (0xA5)
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
#include <RTCDue.h>

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
const byte VSPPin = A7;
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
const char *fileDataSocName = "/dataSOCOCV.txt"; 
const char *fileDataHPPCName = "/dataHPPC.txt"; 
const char *fileDataName = "/data.txt"; 
const char *fileMemoryName = "/memory.txt"; 
unsigned long SDTime = 0.0;
double updateSDTime = 1 * 180 * 1000;
bool SDon = false;
bool SDavailable = false;
/* -- THERMISTANCES -- */
double Thermistances[18] = { -5.5995e-3, 13.7427e-4, -47.1550e-7, // entree du multiplexeur : X0 = TH1 = cable vert
                            18.9321e-3, -26.9060e-4, 117.2023e-7, // entree du multiplexeur : X1 = TH2 = cable orange
                            10.6911e-3, -13.3973e-4, 63.5877e-7, // entree du multiplexeur : X2 = TH3 = cable orange + bic bleu
                            26.6005e-3, -39.7298e-4, 170.3599e-7, // entree du multiplexeur : X3 = TH4 = cable bleu
                            -1.9494e-3, 7.5657e-4, -21.0736e-7, // entree du multiplexeur : X4 = TH5 = cable brun long
                            19.3291e-3, -25.9457e-4, 99.8440e-7 // entree du multiplexeur : X5 = TH6 = cable brun court
                          };
/* -- MEASURES -- */
const int nbBatteries = 11; // nombre de batteries => de 1 à 6 + 1 pour panneux solaires
const int nbCurrent = 2;
double I[nbCurrent] = {};
double V[nbBatteries] = {};
double T[nbBatteries] = {};
double IMean[nbCurrent] = {0.0, 0.0};
double VMean[nbBatteries] = {};
double TMean[nbBatteries] = {};
double SMean[nbBatteries] = {};
double PMean[nbBatteries] = {};
double VSP = 0.0;
double VSPMean = 0.0;
double ISPMean = 0.0;
double PSPMean = 0.0;
int counterMean = 0;
int Vnb = 0;
int Inb = 0;
double Valim = 0.0;
double RV = 32300;
double pot[nbBatteries] = {2950, 2950, 2950, 2950, 2950, 1400, 2950, 2950, 2950, 2950, 2950}; 
double RAlim1 = 50; 
double RAlim2 = 50;
double RT = 4700;
double RSP1 = 1000000;
double RSP2 = 14000;
int nbSamples = 100;
double mvPerI_20 = 31.25;
double offset_20 = 0.36;
double mvPerI_100 = 6.25;
double offset_100 = 1.6;
double tensionTreshold[nbBatteries] = {20, 20, 20, 20, 20, 40, 20, 20, 20, 20, 20};
bool checkAlert[3*nbBatteries] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
/* -- FILTRE KALMANN-- */
char input[] = "{'C1': 1.39e3, 'R0': 4.2e-3, 'R1': 2.8e-3, 'SOCOCV': [-8.05, 16.44, -10.28, 3.38, 11.50], 'dSOCOCV': [-32.22, 49.32, -20.57, 3.38], 'SOC_init': 0.7, 'P_x': [5e-8, 5e-8, 5e-8], 'P_z': [5e-9, 5e-9, 5e-9, 5e-9, 5e-9], 'Q_x': [1e-6, 1e-6, 1e-6], 'Q_z': [1e-10, 1e-10, 1e-10, 1e-10, 1e-10], 'alpha_x': 10, 'betha_x': 1e-3, 'alpha_z': 100, 'betha_z': 5e-4, 'Qn_rated': 400, 'voltage_rated': 12, 'current_rated': 1.3}";
//char input[3000];
double X[2*nbBatteries];
double Z[3*nbBatteries];
double SOCOCV_12[5];
double dSOCOCV_12[4];
double SOCOCV_24[5];
double dSOCOCV_24[4];
double SOCOCV_48[5];
double dSOCOCV_48[4];
double P_x[4*nbBatteries];
double P_z[9*nbBatteries];
double Q_x[4*nbBatteries]; 
double Q_z[9*nbBatteries];
double alpha_x[nbBatteries]; 
double betha_x[nbBatteries]; 
double alpha_z[nbBatteries]; 
double betha_z[nbBatteries]; 
double rho_x = 1;
double rho_z = 1;
double DeltaT; 
double Qn_rated;
double voltage_rated;
double current_rated;
double error = 0.0;
double kalmanTime = 0.0;
double SoC_coulomb[nbBatteries] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
double Capacities[nbBatteries] = {200*3600, 200*3600, 200*3600, 200*3600, 200*3600, 1000*3600, 200*3600, 200*3600, 200*3600, 200*3600, 200*3600};
unsigned long iteration_period = 0;
unsigned long previous_iteration = 0;
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
/* -- CLOCK -- */
RTCDue rtc(XTAL); // Select the Slowclock source
int hours = 0, minutes = 0, seconds = 0; // heure - minute - seconde
int day = 25, month = 7, year = 2022; // jour - mois - année
int hourDay[5], minuteDay[5];
int hourDaySP[9], minuteDaySP[9];
double VH[5*nbBatteries] = {}, IH[5*nbBatteries] = {0}, SH[5*nbBatteries] = {0}, PH[5*nbBatteries] = {0};
double VSPH[9] = {}, ISPH[9] = {}; 
double PSPH[9] = {};
double listheureBat[5] = {1, 4, 8, 12, 16}; 
double listheureSP[9] = {8, 9, 10, 11, 12, 13, 14, 15, 16};
unsigned long TimeUpdateMeasures = 0; 
double updateMeasureTime = 1000;
double normH = 0, normHSP = 0;
bool checkH[5] = {false, false, false, false, false};
bool checkSPH[9] = {false, false, false, false, false, false, false, false, false};
int Hcounter = 0, HSPcounter = 0;
int count_push = 0;
/* -- DIVERS -- */
int mode = 0, oldMode = 0; // default = 0
int counter = 0;
bool on = true; // default = false
double sec = 0.0;
int affichage = 0;
unsigned long previous_millis = 0;  
int  flag_init = 0; // flag utilisé pour l'initialisation de l'arduino et du click (default = -1)
int flag_click = 0, old_flag_click = -1;
bool click_busy = false;
unsigned long clickTime = 0;
char messageClick[2000];
int flag_publish = 1;  // si sur 1 publish activé si sur 0 publish désactivé 
unsigned long previous_millis_publish = 0.0;
bool start = false;
bool initClick = false;
/* -- alertes -- */
int counter_alerts[3*nbBatteries] = {0};

void watchdogSetup(void) {/*** watchdogDisable (); ***/}


void setup() {
  
  /*--- Initialisation des communications serial ---*/
  Serial.begin(115200);
  Serial1.begin(115200);                  

  /*--- Initialisation de la carte SD ---*/
  if(initializeSD()) {
    SDavailable = true;
  }

  /*--- Initialisation LCD ---*/
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(0,0);   
  lcd.print("Starting...");
  
  
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

  /* -- initialisation clock -- */
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

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

  /*--- Enable watchdog ---*/
  
  WDT->WDT_MR = WDT_MR_WDD(0xFFF)
                | WDT_MR_WDRPROC
                | WDT_MR_WDRSTEN
                | WDT_MR_WDV(256 * 13); // Watchdog triggers a reset after 2 seconds if underflow
                                       // 2 seconds equal 84000000 * 2 = 168000000 clock cycles
                                       
  /* Slow clock is running at 32.768 kHz
    watchdog frequency is therefore 32768 / 128 = 256 Hz
    WDV holds the periode in 256 th of seconds  */
  uint32_t status = (RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk) >> 8; // Get status from the last Reset
  Serial.print("RSTTYP = 0b"); Serial.println(status, BIN);  // Should be 0b010 after first watchdog reset
}

void loop() {

  if(flag_init == 0) { // Initialisation des paramètres pour les filtres de Kalmann
    if(SDavailable) {
      //ReadSD(input, fileMemoryName);
    }
    initialisation(input, X, Z, SOCOCV_12, dSOCOCV_12, P_x, P_z, Q_x, Q_z, alpha_x, betha_x, alpha_z, betha_z, &Qn_rated, &voltage_rated, &current_rated); // Pour une batterie
    duplicate(X, Z, P_x, P_z, Q_x, Q_z, alpha_x, betha_x, alpha_z, betha_z, nbBatteries);
    dubble(SOCOCV_24, SOCOCV_12, 5, 2);
    dubble(dSOCOCV_24, dSOCOCV_12, 4, 2);
    dubble(SOCOCV_48, SOCOCV_12, 5, 4);
    dubble(dSOCOCV_48, dSOCOCV_12, 4, 4);
    flag_init = 1;
    /* --- ACTIVATION DU CLICK --- */
    click_busy = true;
    flag_click = 0; 
    old_flag_click = -1;
    clickTime = millis();
  } 
  else if(flag_init == 1) {
      if(mode == 0 && on && millis() - previous_iteration > iteration_period) {
        previous_iteration = millis();
        /* --- Measure VOLTAGE + CURRENT + TEMP ---*/
        takeMeasures(V, I, T, &VSP, nbBatteries, nbCurrent, RV, pot, nbSamples, RT, RAlim1, RAlim2, RSP1, RSP2, offset_20, offset_100, mvPerI_20, mvPerI_100, PMean, &PSPMean, VMean, IMean, TMean, &VSPMean, &counterMean, X, SMean, Thermistances);
        /* --- AFFICHAGE ---*/
        DeltaT = (double)(millis()*0.001 - kalmanTime);
        kalmanTime = (double)(millis()*0.001);       
        //Serial.print("---- DeltaT = "); Serial.println(DeltaT);
        
        /* --- COULOMB Counting ---*/
        for(int i = 0; i < nbBatteries; i++) {
          SoC_coulomb[i] -= (I[0] * DeltaT) / Capacities[i]; // Estimated SoC in %
        }
        /* --- KALMANN FILTER ---*/
        double time1 = micros();
        double time2 = micros();
        for(int i = 0; i < nbBatteries; i++) { // Kalmann filter applied on each 
          if(i == 5) {
             extendedKalmanFilter(I[0], I[0], X + 2*i, Z + 3*i, SOCOCV_48, dSOCOCV_48, V[i], P_x + 4*i, P_z + 9*i, Q_x + 4*i, Q_z + 9*i, &betha_x[i], &betha_z[i], &alpha_x[i], &alpha_z[i], rho_x, rho_z, DeltaT, Capacities[i]);
          } else {
             extendedKalmanFilter(I[0]/5, I[0]/5, X + 2*i, Z + 3*i, SOCOCV_24, dSOCOCV_24, V[i], P_x + 4*i, P_z + 9*i, Q_x + 4*i, Q_z + 9*i, &betha_x[i], &betha_z[i], &alpha_x[i], &alpha_z[i], rho_x, rho_z, DeltaT, Capacities[i]);
          }
          
           //Serial.print("------- Kalman "); Serial.print(i); Serial.print(" => "); Serial.print(micros() - time2); Serial.println(" us.");
           //char text[200];
           //sprintf(text, "SOC = %f", X[2*i]);
           //Serial.print("SOC = "); Serial.println(X[2*i]);
           time2 = micros();
        }

        /* ---Detect if alerts must be sent ---*/
        if(initClick) {
          detect_alertes(nbBatteries, V, X, T, resetThingstreamPin, counter_alerts, checkAlert, initClick, tensionTreshold);
        }
        
        double computationTimeMS = micros() - time1;
        //Serial.print("Total computation time : "); Serial.print(computationTimeMS); Serial.println(" us.");
        if(SDon && SDavailable) {
          printSd(fileDataName, nbBatteries, V, I, T, X, VSP, rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getDay(), rtc.getMonth(), rtc.getYear());
        }
      
    } else if(mode == 1 && on) {

        takeMeasures(V, I, T, &VSP, nbBatteries, nbCurrent, RV, pot, nbSamples, RT, RAlim1, RAlim2, RSP1, RSP2, offset_20, offset_100, mvPerI_20, mvPerI_100, PMean, &PSPMean, VMean, IMean, TMean, &VSPMean, &counterMean, X, SMean, Thermistances);
        
        /* ---- save data to SD card ---- */
        if(SDon && SDavailable) {
          printSd(fileDataHPPCName, nbBatteries, V, I, T, X, VSP, rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getDay(), rtc.getMonth(), rtc.getYear());
        }
        
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
        takeMeasures(V, I, T, &VSP, nbBatteries, nbCurrent, RV, pot, nbSamples, RT, RAlim1, RAlim2, RSP1, RSP2, offset_20, offset_100, mvPerI_20, mvPerI_100, PMean, &PSPMean, VMean, IMean, TMean, &VSPMean, &counterMean, X, SMean, Thermistances);
        /* ---- save data to SD card ---- */
        if(SDon && SDavailable) {
          printSd(fileDataSocName, nbBatteries, V, I, T, X, VSP, rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getDay(), rtc.getMonth(), rtc.getYear());
        }
    }

    if(millis() - buttonTime > updateButtonTime) {
      buttonTime = millis();
      updateLCD(&affichage, &on, &mode, &SDon, V, &Vnb, &Inb, nbBatteries, I, X, T, &stateUP, &test, &stateRIGHT, &stateLEFT, &day, &month, &year, &hours, &minutes, &seconds, &count_push);    
    }

    if(millis() - lcdTime > updateLCDTime) {
      lcdTime = millis();
      hours = rtc.getHours();
      minutes = rtc.getMinutes();
      seconds = rtc.getSeconds();
      day = rtc.getDay();
      month = rtc.getMonth();
      year = rtc.getYear();
      printLCD(affichage, V[Vnb], Vnb, Inb, I[Inb], X[(Vnb)*2], on, mode, SDon, T[Vnb], day, month, year, hours, minutes, seconds);
    }

    if(millis() - SDTime > updateSDTime && SDavailable) {
      SDTime = millis();
      saveSD(fileMemoryName, X, Z, P_x, P_z, Q_x, Q_z, alpha_x, betha_x, alpha_z, betha_z, SOCOCV_12, dSOCOCV_12, Qn_rated, voltage_rated, current_rated);
    }
    
    if(millis() - TimeUpdateMeasures > updateMeasureTime) {
      TimeUpdateMeasures = millis();
      if(updateMeasures(messageClick, rtc.getHours(), nbBatteries, &Hcounter, &HSPcounter, &normH, &normHSP, listheureBat, listheureSP, VMean, IMean, SMean, TMean, PMean, &VSPMean, &PSPMean, &counterMean, V, I, X, VSP, VH, IH, SH, PH, PSPH, VSPH, ISPH, hourDaySP, minuteDaySP, hourDay, minuteDay, &day, &month, &year)) {
        click_busy = true;
        flag_click = 0; 
        old_flag_click = -1;
        clickTime = millis();
      }
    }
    
    if(oldMode != mode) {
      digitalWrite(relayPin,HIGH);
      oldMode = mode;
    }
    
  }

  if(click_busy && (millis() - clickTime > 120000)) {
    activateThingstream(flag_click);
  } else if(click_busy && flag_click != old_flag_click && flag_click < 4) {
    old_flag_click = flag_click;
    activateThingstream(flag_click);
    clickTime = millis();
  } else if(click_busy && flag_click == 4 && initClick) {
    click_busy = false;
    Serial.print("publish = "); Serial.println(messageClick);
    Serial1.println(messageClick);
  } else if(click_busy && flag_click == 4 && !initClick){
    click_busy = false;
    initClick = true;
  }

  char reception[150];
  if(listenToClick(reception)) {
    flag_click += 1;
  }
  WDT->WDT_CR = WDT_CR_KEY(WDT_KEY)
                | WDT_CR_WDRSTT;
}
