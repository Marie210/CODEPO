#define pin_mesure_temperature 1  // Pin Analogique qu'on utilise pour faire la mesure de température
#include <extendedKalmanFilter.h> 


/* ----- VARIABLEs DECLARATION ----- */
int mode = 0;
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
bool test = true;
double cat = 8; 
double Q_consumed = 0.0;
double SoC_coulomb;
double seconds = 0.0;

unsigned long previous_millis = 0 ;  
int R1 = 10000; // Valeur de la résistance avec laquelle on fait la mesure de la température
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
float logR2, R2, T;
int  flag_init = 0; // flag utilisé pour l'initialisation de l'arduino et du click 
int flag_publish = 1;  // si sur 1 publish activé si sur 0 publish désactivé 


void setup() {
  Serial.println("---- initialisation ----");
  Serial.begin(115200);
  Serial1.begin(115200);
  
  delay(1000);
}

void loop() {

  delay(10);

  if(flag_init == 4) {
    char input[] = "{'C1': 1.704510181209152, 'C2': 1.528807876468525, 'R0': 0.045060113042053, 'R1': 0.022227266870883, 'R2': 0.041772924359798, 'SOCOCV': [-5899.08970208633, 33676.8099461629, -83441.9838542838, 117439.916788238, -103158.271786075, 58466.9571460381, -21320.9039215880, 4810.53130689387, -602.846517986747, 26.7902085666153, 3.13908872646908, 5.40144738302999], 'dSOCOCV': [-64889.9867229497, 336768.099461629, -750977.854688554, 939519.334305904, -722107.902502523, 350801.742876229, -106604.519607940, 19242.1252275755, -1808.53955396024, 53.5804171332306, 3.13908872646908], 'SOC_init': 0.7, 'P_x': [ 5e-7, 0, 0, 0, 5e-7, 0, 0, 0, 5e-7 ], 'P_z': [ 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8, 0, 0, 0, 0, 0, 5e-8], 'Q_x': [ 1e-10, 0, 0, 0, 1e-10, 0, 0, 0, 1e-10 ], 'Q_z': [ 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8, 0, 0, 0, 0, 0, 1e-8 ], 'R_x': 1e-5, 'R_z': 1e-7, 'DeltaT': 0.46, 'Qn_rated': 1300}";
    initialisation(input, X, Z, SOCOCV, dSOCOCV, P_x, P_z, Q_x, Q_z, &R_x, &R_z, &DeltaT, &Qn_rated);
    flag_init = 5;
  }

  if(flag_init == 5) {


    /* --- Measure VOLTAGE + CURRENT ---*/
    measureVoltage(&Vt_Actual, &U);

    if(Vt_Actual != 0 || U != 0) {
      /* --- COULOMB Counting ---*/
      Q_consumed += U * DeltaT; // current * time; => Q_consumed [A.s]
      SoC_coulomb = 1.0 - (Q_consumed) / Qn_rated; // Estimated SoC in %
  
      /* --- AFFICHAGE ---*/
      double sec = millis()*0.001;
      Serial.print(sec, 5); Serial.print("  T;  ");
      Serial.print(Vt_Actual, 5); Serial.print("  VT;  ");
      Serial.print(U, 5); Serial.print("  U;  ");
      Serial.print(X[0], 5); Serial.print("  S;  ");
      Serial.print(SoC_coulomb, 5); Serial.print("  C;  ");
  
      
      /* --- KALMAN FILTER ---*/
      error = extendedKalmanFilter(U, X, Z, SOCOCV, dSOCOCV, Vt_Actual, P_x, P_z, Q_x, Q_z, R_x, R_z, DeltaT, Qn_rated);
      Serial.print("\n");
    }
    
    //String message = createData(Vt_Actual, U, X, Z, error);
    if (millis()  >=  previous_millis + 100000 && flag_publish == 1) {
      previous_millis =  millis();
      Serial.print("publish : ");
      Serial.println(previous_millis) ;
      publish(Vt_Actual, U, X, Z, error);
    }

  }
  else {
     flag_init = initThingstream(flag_init);
  }

  if (Serial1.available ()){
      
      //char st =  Serial1.read();
      //Serial.println(st);
      /*
      if (analyse(st) ==  0){
        flag_publish = 0;
      }
      if (analyse(st) ==  1){
        flag_publish = 1;
      }
      */
   }

}
