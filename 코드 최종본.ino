
#include "string.h"
#include <MsTimer2.h>
#include "U8glib.h"
#include <Stepper.h>

//그래픽 LCD의 핀번호
U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8);    // SPI Com: SCK = 13, MOSI = 11, CS = 10, CD = 9, RST = 8
//스테핑 모터의 핀번호
int steps = 10;
Stepper stepper0(steps, 3,4,5,6);
int sSpeed = 1700;
int myDelay = 10;

//칫솔을 감지하는 초음파 센서의 핀번호
int tooth_echoPin = 42;
int tooth_trigPin = 43;
//종이컵을 감지하는 초음파 센서의 핀번호
int cup_echoPin = 46;
int cup_trigPin = 47;
//남은 물의 양을 측정하는 초음파 센서의 핀번호
int water_trigPin = 32;
int water_echoPin = 33;
//워터펌프의 핀번호
int water_pump_pin = 34;

int viration = 22;

int Bottle = 100;     //물통의 크기
int pumb_flag_cur = 0;
int pumb_flag_prev = 0;
int Timer;
int num = 0;
int vir = 1;

void draw(int num,int water){
  u8g.setFont(u8g_font_unifont);

  u8g.setPrintPos(0, 10);
  u8g.print("water:  ");
  u8g.setPrintPos(50, 10);
  u8g.print(water);
  u8g.setPrintPos(70, 10);
  u8g.print("%");

if(num >= 0){
  u8g.drawStr( 60, 40, "sec : ");
  u8g.setPrintPos( 110, 40);
  u8g.print(num);
  if(num == 0) {
    u8g.drawStr( 60, 55, "Complete!!");
    if(vir == 0){
      for(int i = 0; i < 5; i++){
    digitalWrite(viration, HIGH);
    delay(200);
    digitalWrite(viration, LOW);
      }
    vir = 1;
    }
  }
 }
}

void ISR_Timer(){
  Timer++;
  if(num > 0){
  if(Timer>1000){
    Serial.println(num);
    num--;
    Timer = 0;
  }
  }
}

void setup() {
  // put your setup code here, to run once:
  
  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();// rotate screen, if required
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);
 
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
    
  stepper0.setSpeed(sSpeed);
 // Serial.begin(9600);

  pinMode(viration, OUTPUT);
  pinMode(tooth_trigPin, OUTPUT);
  pinMode(tooth_echoPin, INPUT);
  pinMode(cup_trigPin, OUTPUT);
  pinMode(cup_echoPin, INPUT);
  
  pinMode(water_pump_pin,OUTPUT); 
  pinMode(water_trigPin,OUTPUT); 
  pinMode(water_echoPin,INPUT); 

  digitalWrite(water_pump_pin,HIGH); 
  delay(100);

  Serial.println("pump off");
}

void loop() 
{
 static int water;
 float tooth_duration, tooth_distance;
 float cup_duration, cup_distance;

  //칫솔의 유무를 측정
  digitalWrite(tooth_trigPin, HIGH);
  delay(500);
  digitalWrite(tooth_trigPin, LOW);

  tooth_duration = pulseIn(tooth_echoPin, HIGH); 
  tooth_distance = ((float)(340 * tooth_duration) / 10000) / 2;
  Serial.print("tooth_distance : ");
  Serial.println(tooth_distance);

  //칫솔이 가까이 오면 스테핑모터를 작동
  if(tooth_distance < 5){
    stepper0.step(150);
    num = 5;
    vir = 0; 
  }

  u8g.firstPage(); 
      do {
       MsTimer2::set(1,ISR_Timer);      //1ms, ISR_Timer
       MsTimer2::start();
       Serial.begin(9600);    //Serial Baudrate Set
        draw(num, water);
      } while( u8g.nextPage() );
      // rebuild the picture after some delay
      delay(500);

  //종이컵의 유무를 측정
  digitalWrite(cup_trigPin,LOW); 
  delayMicroseconds(2); 
  digitalWrite(cup_trigPin,HIGH); 
  delayMicroseconds(10); 
  digitalWrite(cup_trigPin,LOW); 

  cup_duration = pulseIn(cup_echoPin, HIGH); 
  cup_distance = (cup_duration/2)/29.1;
  Serial.print("cup_distance : ");
  Serial.println(cup_distance);
  
  if(cup_distance < 5){
    pumb_flag_cur = 1;
    if(pumb_flag_cur==1 && pumb_flag_prev==0){
    Serial.println("pump on");
    
    delay(1000);
    for(int i=0;i<8;i++){
      digitalWrite(water_pump_pin, LOW); 
      delay(420);
      digitalWrite(water_pump_pin, HIGH);
    }
    delay(1000);
    
    Serial.println("pump off");
  }
  
  delay(100);
  
      long water_duration, water_distance;
      
      //남은 물의 양 측정
      digitalWrite(water_trigPin, HIGH);
      delay(500);
      digitalWrite(water_trigPin, LOW);

      water_duration = pulseIn(water_echoPin, HIGH); 
      water_distance = ((float)(340 * water_duration) / 10000) / 2;
      water = (Bottle - water_distance) * (100 / Bottle);
      Serial.print("water : ");
      Serial.println(water);
      // picture loop
      
   u8g.firstPage(); 
     do {
      draw(num, water);
      } while( u8g.nextPage() );
      delay(500);
  }

  pumb_flag_prev = pumb_flag_cur;
  pumb_flag_cur = 0;
  
    }
