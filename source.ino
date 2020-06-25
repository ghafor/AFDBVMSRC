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

bool is_backward = false;
bool edit_status;
char motor_state = 'S';
double RR;

const int buzzer = 13;
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

void setup() {
 startTimer3(1000);
  //Open roboclaw serial ports
  roboclaw.begin(38400);

  u8g.setFont(u8g_font_6x13);
  u8g.setColorIndex(1);    
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  pinMode(12,OUTPUT);
  while(current_angle > 4 || _switch == 1){
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
  set_button = digitalRead(14);
  if (set_button==1){
    read_input_amounts();//read inputs
    }else {
      edit_status = false;//show edit off  
    }
  _switch = digitalRead(21);
  map_angle();
  set_ratio_case();
  switch (motor_state) {
    //go forward
    case 'F':
      move_to(cc_vol);
      
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
if(charge_indicator <65) {
  digitalWrite(12,HIGH);
}else if(charge_indicator >82) {
  digitalWrite(12,LOW);
}

//pressure




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
  if (current_angle > 4 || _switch == 1) {
    roboclaw.BackwardM1(address, motor_speed);
  } else {
    motor_state = 'S';
    roboclaw.BackwardM1(address, 0);
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



void buzzer_func() {
  tone(buzzer, 5);
  delay(50);
  noTone(buzzer);
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
  cc_vol = map(cc_vol,0,1023,15,80);
  
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
    u8g.drawVLine(65,0,128);
    u8g.drawRFrame(0,0,128,64,4);            // upper frame
  // convert floats into char u8g strings 
//  char temp_string[5];
//  dtostrf(1023.4, 3, 1, temp_string);   
//  u8g.drawStr( 15, 13, temp_string);       // do this for temperature
  //Breath Per Minute Display
  u8g.drawStr( 5, 13, "BPM");
 
  String _BPM = String(no_breath);
  u8g.drawStr( 40, 13, _BPM.c_str());

  //Inspiration to Expiration Ratio display
   u8g.drawStr( 5, 25, "I:E");
  u8g.drawStr(40, 25, "1:");


 char buf[2];
  snprintf (buf, 2, "%d", E);
  u8g.drawStr(53, 25, buf);
  
  //volume
  u8g.drawStr( 5, 37, "TV");
 
  String _v = String(cc_vol);
  u8g.drawStr( 40, 37, _v.c_str());

  //mode
  u8g.drawStr( 5, 49, "Mode");
  u8g.drawStr( 40, 49, "A");

//edit status
u8g.drawStr( 70, 13, "Edit");
//  lcd.setCursor(13, 1);
  if (!edit_status) {
    u8g.drawStr( 110, 13, "OFF");
  } else {
    u8g.drawStr( 110, 13, "ON");
  }
//show charge
  u8g.drawStr( 70, 25, "Charge");
  String _ci = String(charge_indicator);
  u8g.drawStr( 110, 25, _ci.c_str());

  //show pressure
  u8g.drawStr( 70, 38, "Press");
  //String _ci = String(charge_indicator);
  u8g.drawStr( 110, 38, "25");
}
