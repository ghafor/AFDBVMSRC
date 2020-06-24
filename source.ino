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
U8GLIB_ST7920_128X64 u8g(13, 11, 12, U8G_PIN_NONE);


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
  //  display_welcome_text();
//    set_default_values();

  //Open roboclaw serial ports
  roboclaw.begin(38400);

  u8g.setFont(u8g_font_6x13);
  u8g.setColorIndex(1);    
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  pinMode(12,OUTPUT);
  //zero position at the start
  while(current_angle > 4 || _switch == 1){
    zero();
  }
  
  motor_state = 'F';
  timer_counter = 0;
 
}
//states: 
// F = Forward, B = Backward, S = Stop
void loop() {
  
  u8g.firstPage();
  do {   
    draw();
    display_up_time();
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
if(charge_indicator <80) {
  digitalWrite(12,HIGH);
}else if(charge_indicator >95) {
  digitalWrite(12,LOW);
}
lcd.setCursor(1, 1);
lcd.print(charge_indicator+"%");


}
//because the motor does not have encoder, for the start of the work we have set the motor backward movement speed manual. 
//max speed = 127, min speed =  0.
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
//stop the motors
void stop_motors() {
//    lcd.setCursor(1, 1);
//  lcd.print(timer_counter/100);
  if (timer_counter < time_per_BPM) {
    roboclaw.BackwardM1(address, 0);
  } else {
    motor_state = 'F';
    timer_counter = 0;
  }
}
//zero the position of the gears
void zero() {
  if (current_angle > 4 || _switch == 1) {
    roboclaw.BackwardM1(address, motor_speed);
  } else {
    motor_state = 'S';
    roboclaw.BackwardM1(address, 0);
  }
}
//move to the wanted angle
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

void display_data() {
  //show motor_speed of motor to set air flow pressure
  lcd.setCursor(6, 0);
  lcd.print("I:E 1:");
  lcd.print(E);
  lcd.print(" ");

  //show cc volume
  lcd.setCursor(6, 1);
  lcd.print("V ");
  lcd.print(cc_vol);

  lcd.setCursor(0, 0);
  lcd.print("B ");
  lcd.print(no_breath);
  lcd.print(" ");
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

void display_welcome_text() {
  //write welcome
  lcd.setCursor(0, 0);
  lcd.print("Afghan Dreamers");
  lcd.setCursor(1, 0);
  lcd.print("   ventilator   ");
  delay(800);
  //clear screen
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(1, 0);
  lcd.print("                ");
}
void display_edit_status(int state) {
  lcd.setCursor(14, 0);
  lcd.print("E");
  lcd.setCursor(13, 1);
  if (state == 0) {
    lcd.print("OFF");
  } else {
    lcd.print("ON ");
  }
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
  u8g.drawStr( 5, 37, "V");
 
  String _v = String(cc_vol);
  u8g.drawStr( 40, 37, _v.c_str());

  //mode
  u8g.drawStr( 5, 49, "Mode");
  u8g.drawStr( 40, 49, "A");

//edit status
u8g.drawStr( 70, 13, "E");
  lcd.setCursor(13, 1);
  if (!edit_status) {
    u8g.drawStr( 90, 25, "OFF"));
  } else {
    u8g.drawStr( 90, 25, "ON"));
  }
}
