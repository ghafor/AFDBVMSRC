#include <Timer3.h>
#include "LiquidCrystal.h"
#include "U8glib.h"
//See BareMinimum example for a list of library functions
//Includes required to use Roboclaw library
#include <SoftwareSerial.h>
#include "RoboClaw.h"

//See limitations of Arduino SoftwareSerial
SoftwareSerial serial(10, 11);
RoboClaw roboclaw(&serial, 10000);
U8GLIB_ST7920_128X64 u8g(9, 7, 8, U8G_PIN_NONE);


#define address 0x80
#define min_angle 590
#define max_angle 160
int an=0;
bool is_backward = false;
bool edit_status;
bool is_error;
bool is_triggered;
bool test = true;
char motor_state = 'S';
double RR;

const int buzzer = 13;
float current_pres;
float peep;
int motor_speed;
int rr_vol;
int time_per_BPM = 0;
int no_breath;
int _switch;
int set_button;
int cc_vol;
int cc;
int angle_vol;
int current_angle;
int timer_counter = 0;
int E;
int charge_indicator;
unsigned long prevMillis;
unsigned long currentMillis;
unsigned long inspirationTime;

String error_text;
void setup() {
 startTimer3(1000);
  //Open roboclaw serial ports
  roboclaw.begin(38400);

  u8g.setFont(u8g_font_6x10);
  u8g.setColorIndex(1);    
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  pinMode(12,OUTPUT);
  while(current_angle > 4){
    zero();
  }
  
  motor_state = 'F';
  timer_counter = 0;
 
}

void loop() {
  
  u8g.firstPage();
  do {   
    draw();
   
  } while( u8g.nextPage() );
 
  //reading data from volumes
  read_input_amounts();//read inputs
  set_button = digitalRead(14);

  _switch = digitalRead(21);
  map_angle();
  set_ratio_case();
  switch (motor_state) {
    //go forward
    case 'F':
      //if(test == true and current_pres<7) {
        an=0;
   an=current_angle;
        move_to(cc_vol);
       //}else{motor_state='B';}
      break;
    case 'B':
      zero();
      
      break;
    case 'S':
      stop_motors();
      
      break;
  }

//charge indicator setup
charge_indicator = analogRead(A12);
charge_indicator = map(charge_indicator,0,1023,0,100);
if(charge_indicator <90) {
  digitalWrite(12,HIGH);
}else if(charge_indicator >82) {
  digitalWrite(12,LOW);
}

//pressure
readPressure();
if(current_pres <-1.0) {
  //breath triggered
  motor_state = 'F';
}

if(current_pres>4) {
  test = false;
}


//alarm condition
alarmConditions();
}

void set_ratio_case() {
  switch(E) {
    case 1: 
      motor_speed = 127;
      break;
    case 2:
      motor_speed = 107;
    break; 
    case 3: 
    motor_speed = 87;
    break;
  }
}
void stop_motors() {
  if (timer_counter < time_per_BPM) {
    roboclaw.BackwardM1(address, 0);
  } else {
    motor_state = 'F';
    timer_counter = 0;
  }
}
void zero() {
  if (current_angle > 15) {
    roboclaw.BackwardM1(address, motor_speed);
  } else {
    motor_state = 'S';
    roboclaw.BackwardM1(address, 0);
    peep = current_pres;
  }
  
}
void  move_to (int angle) {
  if (angle > current_angle) {
    roboclaw.ForwardM1(address, 127);
  } else {
    motor_state = 'B';

  }
}

void calculate_breath_peroid(int volume) {
  // no_breath = map(volume,1023,0,7,41);
  //  RR = 60 / ((volume / 32) + 7);
  //  RR *= 1000;
  //calculate number of breathing
  no_breath = (volume / 32) + 7;
  time_per_BPM = 60000 / no_breath;
}




void set_default_values() {
  motor_speed = 127;
  calculate_breath_peroid(1023);
  
}


void read_input_amounts() {
  //motor motor_speed
  E = analogRead(A15);
  E = map(E, 0, 1023, 1, 3);
  //delay breath
  rr_vol = analogRead(A13);
  calculate_breath_peroid(rr_vol);
  //cc tidal volume
  cc_vol = analogRead(A7);
  cc_vol = map(cc_vol,0,1023,20,80);
  
  edit_status = true;
  
}
int _min = 600;

void map_angle() {
  angle_vol = analogRead(A10);
  current_angle = map(angle_vol, 590, 160, 0, 100);
}

 ISR(timer3Event) {
  startTimer3(1000);
  timer_counter++;
}

void draw(){
    u8g.drawVLine(65,0,45);
    u8g.drawHLine(0,45,128);
    u8g.drawRFrame(0,0,128,64,2);            // upper frame
  u8g.drawStr( 5, 10, "BPM");
 
  String _BPM = String(no_breath);
  u8g.drawStr( 40, 10, _BPM.c_str());

  //Inspiration to Expiration Ratio display
   u8g.drawStr( 5, 22, "I:E");
  u8g.drawStr(40, 22, "1:");


 char buf[2];
  snprintf (buf, 2, "%d", E);
  u8g.drawStr(53, 22, buf);
  
  //volume
  u8g.drawStr( 5, 34, "TV");
 
  String _v = String(cc_vol);
  u8g.drawStr( 40, 34, _v.c_str());

  //mode
  u8g.drawStr( 5, 44, "Mode");
  u8g.drawStr( 40, 44, "A");

//edit status
u8g.drawStr( 70, 10, "An");
    String _current_angle = String(an);
    u8g.drawStr( 100, 10,_current_angle.c_str());
 
//show charge
  u8g.drawStr( 70, 22, "Charge");
  String _ci = String(charge_indicator);
  u8g.drawStr( 110, 22, _ci.c_str());

  //show pressure
  u8g.drawStr( 70, 34, "Pre");
  String _current_pres = String(current_pres);
  u8g.drawStr( 96, 34, _current_pres.c_str());
//show peep
  u8g.drawStr( 70, 43, "peep");
  String _peep = String(peep);
  u8g.drawStr( 96, 43, _peep.c_str());
  
  //if pressure is so high, if charge is low, if something went wrong, run the buzzer
   if(charge_indicator < 65) {
    u8g.drawStr( 5, 58, "battery critical low");
   }else if(current_pres > 20) {
    u8g.drawStr( 5, 58, "Critical Pressure");
   }else {
        u8g.drawStr( 5, 58, "System is OK!");
    }
}

void alarmConditions() {
   //if pressure is so high, if charge is low, if something went wrong, run the buzzer
   if(charge_indicator < 65 && charge_indicator>23) {
    buzzer_func(1);
    //u8g.drawStr( 5, 58, "battery critical low");
   }else if(current_pres > 20) {
    //u8g.drawStr( 5, 58, "Critical Pressure");
    buzzer_func(2);
    motor_state = 'B';
   }else {
      //  u8g.drawStr( 5, 58, "System is OK!");
    }
}
void buzzer_func(int state) {
  switch(state){
    //if battery charge is under 20 percent
    case 1:
      for(int i=0;i<3;i++) {
        tone(buzzer, 5);
        delay(50);
      }
      noTone(buzzer);
    break;
    //pressure overload condition
    case 2:
      for(int i=0;i<5;i++) {
        tone(buzzer, 7);
        delay(40);
      }
      noTone(buzzer);
    break;
    
  }
  }

void readPressure() {
     // read the voltage
   int V = analogRead(A9);  // sensor Vout in (0-1023)
    int Vs = 1023;  // sensor Vsupply in (0-1023)

    float Pmin = -2;        // pressure min in Psi
    float Pmax = 2;         // pressure max in Psi

    float Vmax = 1023;        // max voltage in range from analogRead

    // conversion from V to cmH2O pressure
    float Vs_A = (5*Vs/Vmax); // sensor Vsupply in V
    float Vout = (5*V/Vmax);  // sensor Vout in V

    // pressure difference between port 1 and 2 of the sensor in Psi
    // based on the TE Connectivity calibration curve for MS4525 002 sensor

    float P = ((Vout - 0.5)*(Pmax - Pmin)/(0.8*Vs_A)) + Pmin;

    // convert to cmH2O
    P *= 70.307;        // cmH2O (1 Psi = 70.307 cmH2O)
    P += 0.17;
    Serial.println(P);
    current_pres = P;
} 
