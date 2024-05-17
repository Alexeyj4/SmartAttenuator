#define OLED_DISPLAY_TYPE 1106
#include <Oled.h>
#include <ESP32Encoder.h>

#define BUZZER_PIN 13
#define LED_PIN 14
#define SI_3V_PIN 16
#define LE_3V_PIN 17
#define SH_CLK_3V_PIN 18
#define RESET_3V_PIN 19
//#define OLED_SDA_PIN 21
//#define OLED_SCL_PIN 22
#define ENC_BTN_PIN 25
#define ENC_B_PIN 26
#define ENC_A_PIN 27

Oled oled;
ESP32Encoder enc;

void setup() {
  // put your setup code here, to run once: 
  Serial.begin(115200);
  oled.begin(); 
  pinMode(LED_PIN,OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  pinMode(ENC_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  enc.attachFullQuad(ENC_A_PIN, ENC_B_PIN);

}

void beep(){
  for(int i=0;i<200;i++){    
    digitalWrite(BUZZER_PIN,1);
    delay(1);
    digitalWrite(BUZZER_PIN,0);    
    delay(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN,1);
  delay(500);
  digitalWrite(LED_PIN,0);
  delay(500);
  int enc_pos=-floor(enc.getCount()/2);
  Serial.println(enc_pos);
  oled.print(0,String(enc_pos));  
  oled.update();
  
  if(digitalRead(ENC_BTN_PIN)==0){
    Serial.println("key pressed");
    beep();
  }
  
}
