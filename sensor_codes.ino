void setup() {
  	// initiating data rate of 9600 bps
	Serial.begin(9600);
}

void loop() {
  	// repeatedly calling read function
  	read();
}


void read() {
    // read the voltage

    int V = analogRead(A0); 	// sensor Vout in (0-1023)
    int Vs = analogRead(A1);	// sensor Vsupply in (0-1023)

    float Pmin = -2;   			// pressure min in Psi
    float Pmax = 2;    			// pressure max in Psi

    float Vmax = 1023;     		// max voltage in range from analogRead

    // conversion from V to cmH2O pressure
    float Vs_A = (5*Vs/Vmax);	// sensor Vsupply in V
    float Vout = (5*V/Vmax);	// sensor Vout in V

    // pressure difference between port 1 and 2 of the sensor in Psi
    // based on the TE Connectivity calibration curve for MS4525 002 sensor

    float P = ((Vout - 0.5)*(Pmax - Pmin)/(0.8*Vs_A)) + Pmin;

    // convert to cmH2O
    P *= 70.307; 				// cmH2O (1 Psi = 70.307 cmH2O)
  
    Serial.println(P);
    delay(200);
  }
