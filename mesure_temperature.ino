#define pin_mesure_tension 0  // Pin Analogique qu'on utilise pour faire la mesure de tension
#define pin_mesure_temperature 1  // Pin Analogique qu'on utilise pour faire la mesure de température

int R1 = 10000; // Valeur de la résistance mise en série avec la résistance qui permet de faire la mesure de température
float A = 0.000623, B = 0.000349, C = -0.0000005648; // Les coefficients de Steinhart-Hart obtenus expérimentalement pour notre thermistance
float logR_th, T;

float Vin = 5;
float pas = Vin / 1024; // L'arduino pour ces mesures découpe 5V en 1024 valeurs discrètes


void setup() {
  Serial.begin(9600);
  Serial.print("Le code commence.\n");
}


void loop() {

  Serial.print("######## Boucle ############################\n");
  
  /* ############ Mesure de tension ########################################## */ 
  int mesure_tension = analogRead(pin_mesure_tension);  // Lis la tension en un format digital compris entre 0 et 1023 (découpe 5V en 1024 parts égales)
  float tension = mesure_tension * pas; // Pour obtenir la tension sous format analogique il faut la multiplier par le pas

  // Affichage de la tension mesurée
  Serial.print("Tension : ") ;
  Serial.print(tension); 
  Serial.print(" Volts\n");

  // Calcul de la résistance
  float R_th = R1 * ( (Vin/tension) - 1);
  Serial.print("Résistance de la thermistance : ");
  Serial.print(R_th); 
  Serial.print(" Ohms\n");


  /* ############ Mesure de température ########################################## */ 
  int mesure_temperature = analogRead(pin_mesure_temperature);
  logR_th = log(R_th);
  T = (1.0 / (A + B*logR_th + C*logR_th*logR_th*logR_th)); // Formule de Steinhart-Hart liant la résistance mesuré à la température (en Kelvin)
  T = T - 273.15; // Conversion Kelvin -> Celsius
  
  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.println(" degre Celsius\n");

  
  delay(2000) ;
}
