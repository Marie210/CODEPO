#define WDT_KEY (0xA5)
#include <extendedKalmanFilter.h> 


/* ----- VARIABLEs DECLARATION ----- */
int mode = 1;
int counter = 0;
bool on = false;
bool check = true;
int HPPC_time = 0.0;
const char *filename = "/data.txt"; 
double parameters[5]; // = [C1, C2, R0, R1, R2]
double U = 0;
double X[3];
double Z[5];
double SOCOCV[12];
double dSOCOCV[11];
double Vt_Actual = 0.0;
double P_x[9];
double P_z[25];
double Q_x[9]; 
double Q_z[25];
double R_x;
double R_z;
double DeltaT; 
double Qn_rated;
double error = 0.0;
double cat = 8; 
double Q_consumed = 0.0;
double SoC_coulomb;
double seconds = 0.0;
double temperature;
double sec = 0.0;

unsigned long previous_millis = 0 ;  
int  flag_init = 4; // flag used for the initialisation of the click 
int flag_publish = 1;  // if equals to 1 publish activated, if equals to 0 publish disabled
unsigned long previous_millis_publish = 0.0;

void watchdogSetup(void) {/*** watchdogDisable (); ***/}


void setup() {
  Serial.println("---- initialisation ----");
  Serial.begin(115200);
  Serial1.begin(115200);

  /*--- Enable watchdog ---*/
  
  WDT->WDT_MR = WDT_MR_WDD(0xFFF)
                | WDT_MR_WDRPROC
                | WDT_MR_WDRSTEN
                | WDT_MR_WDV(256 * 60); // Watchdog triggers a reset after 2 seconds if underflow
                                       // 2 seconds equal 84000000 * 2 = 168000000 clock cycles
  
  /* Slow clock is running at 32.768 kHz
    watchdog frequency is therefore 32768 / 128 = 256 Hz
    WDV holds the periode in 256 th of seconds  */
  uint32_t status = (RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk) >> 8; // Get status from the last Reset
  Serial.print("RSTTYP = 0b"); Serial.println(status, BIN);  // Should be 0b010 after first watchdog reset

  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
}

void loop() {

  if(mode == 0) {
      delay(10);
  
     //initalize Kalman filter with starting data
      if(flag_init == 4) {
        char input[] = "{'C1': 1.704510181209152, 'C2': 1.528807876468525, 'R0': 0.045060113042053, 'R1': 0.022227266870883, 'R2': 0.041772924359798, 'SOCOCV': [-5899.08970208633, 33676.8099461629, -83441.9838542838, 117439.916788238, -103158.271786075, 58466.9571460381, -21320.9039215880, 4810.53130689387, -602.846517986747, 26.7902085666153, 3.13908872646908, 5.40144738302999], 'dSOCOCV': [-64889.9867229497, 336768.099461629, -750977.854688554, 939519.334305904, -722107.902502523, 350801.742876229, -106604.519607940, 19242.1252275755, -1808.53955396024, 53.5804171332306, 3.13908872646908], 'SOC_init': 0.0, 'P_x': [ 5e-7, 0, 0, 0, 5e-7, 0, 0, 0, 5e-7 ], 'P_z': [ 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8], 'Q_x': [ 1e-10, 0, 0, 0, 1e-10, 0, 0, 0, 1e-10 ], 'Q_z': [ 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8 ], 'R_x': 1e-5, 'R_z': 1e-7, 'DeltaT': 0.46, 'Qn_rated': 1300}";
        initialisation(input, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, &R_x, &R_z, &DeltaT, &Qn_rated);
        flag_init = 5;
      }
    
    //once everything is initialized the code takes measures
      if(flag_init == 5) {
    
    
        /* --- Measure VOLTAGE + CURRENT ---*/
        measureVoltage(&Vt_Actual, &U);
        temperature = mesureTemperature();
    
        /* --- AFFICHAGE ---*/
        
        sec = millis()*0.001;
        Serial.print(sec, 5); Serial.print("  Ti;  ");
        Serial.print(temperature, 5); Serial.print("  Te;  ");
        Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
        Serial.print(error, 5); Serial.print("  E;  ");
        Serial.print(U, 5); Serial.print("  U;  ");
        Serial.print(X[0], 5); Serial.print("  S;  ");
        Serial.print(SoC_coulomb, 5); Serial.print("  C;  ");
        Serial.print("\n");
        
    
        if(Vt_Actual != 0 && U != 0) {
          /* --- COULOMB Counting ---*/
          Q_consumed += U * DeltaT; // current * time; => Q_consumed [A.s]
          SoC_coulomb = 0.0 - (Q_consumed) / Qn_rated; // Estimated SoC in %
      
          /* --- KALMAN FILTER ---*/
          error = extendedKalmanFilter(U, X, Z, SOCOCV, dSOCOCV, Vt_Actual, P_x, P_z, Q_x, Q_z, R_x, R_z, DeltaT, Qn_rated);
        }
        
        //String message = createData(Vt_Actual, U, X, Z, error);
        if (millis()  >=  previous_millis + 100000 && flag_publish == 1) {
          //previous_millis =  millis();
          //Serial.print("publish : ");
          //Serial.println(previous_millis) ;
          //publish(Vt_Actual, U, X, Z, error);
        }
    
      }
      else {
         initThingstream(&flag_init);
      }
  } else if(mode == 1 && check == true) {
    delay(10);

    if(Vt_Actual < 1.6 && counter == 8) {
      check = false;
      counter += 1;
      digitalWrite(2,HIGH);
    } else {
      measureVoltage(&Vt_Actual, &U);      
      Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
      Serial.print(U, 5); Serial.print("  U;  ");
      Serial.print(sec, 5); Serial.print("  Ti;  ");
      Serial.print("\n");
      
      sec = millis()*0.001;
      if(sec-previous_millis > HPPC_time) {
        previous_millis = sec;
        on = !on;
        if(on) {
          digitalWrite(2,HIGH);
          HPPC_time = 600;
        } else {
          digitalWrite(2,LOW);
          HPPC_time = 30;
        }
      }
    }
  }

  
  WDT->WDT_CR = WDT_CR_KEY(WDT_KEY)
                | WDT_CR_WDRSTT;
}
