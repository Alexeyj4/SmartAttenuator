#define OLED_DISPLAY_TYPE SH1106
#include <Oled.h>

#define BUZZER_PIN 13
#define LED_PIN 14
#define SI_3V_PIN 16
#define LE_3V_PIN 17
#define SH_CLK_3V_PIN 18
#define RESET_3V_PIN 19
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define ELED_ENC_BTN_PIN 25
#define ELED_ENC_B_PIN 26
#define ELED_ENC_A_PIN 27

Oled oled;

void setup() {
  // put your setup code here, to run once:
  oled.begin();
  delay(100);
  oled.clear();
  oled.print(3,"Проверка");
  oled.update(); 
  
  pinMode(LED_PIN,OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN,1);
  delay(500);
  digitalWrite(LED_PIN,0);
  delay(500);
  oled.print(2,"тест");

}
