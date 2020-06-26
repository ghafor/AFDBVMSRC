void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  read();
}


void read() {
    // read the voltage
    int V = analogRead(A0); 
    int Vs = analogRead(A1);
    float Pmin = -2;   // pressure max in mbar
    float Pmax = 2;    // pressure min in mbar
    float Vmax = 1023;     // max voltage in range from analogRead
    // convert to pressure
    float Vs_A = (5*Vs/Vmax);
    float pres = ((5*V/1023) -0.5)*(Pmax - Pmin)/(0.8*Vs_A) + Pmin;//psi
    pres *= 70.307; // cmH2O
    // convert to cmH20
  
    Serial.println(Vs_A);
    delay(200);
  }
