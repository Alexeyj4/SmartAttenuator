#define BUZZER_MODE_ON //закомментируйте, если нужен беззвучный режим
#define BT_MODULE_ON //закомментируйте, если нужен режим без BT (меньше энергопотребление)
//в библиотеке BLE.h задайте имя BT
#define OLED_DISPLAY_TYPE 1106
#include <Oled.h>
#include <ESP32Encoder.h>
#include "attValues.h"
#include <EEPROM.h>
#ifdef BT_MODULE_ON
#include <Ble.h>
#endif

#define BUZZER_PIN 13
#define LED_PIN 14
#define SI_3V_PIN 16
#define LE_3V_PIN 17
#define SH_CLK_3V_PIN 18
#define RESET_3V_PIN 19
//#define OLED_SDA_PIN 21 //ноги порта I2C определяются 21 и 22 в библиотеке по-умолчанию
//#define OLED_SCL_PIN 22 //ноги порта I2C определяются 21 и 22  в библиотеке по-умолчанию
#define ENC_BTN_PIN 25
#define ENC_B_PIN 26
#define ENC_A_PIN 27
#define updateRegisterInterval 5000 //каждые n сек обновляется регистр микросхемы аттенюатора (на случай помех и т.п.)

signed char prev_enc_pos; //предыдущая позиция энкодера для фиксации факта поворота
signed char enc_pos; //текущая позиция энкодера
bool button_released_flag=1; //флаг, была ли отжата кнопка после нажатия
long prevMillis=millis();

Oled oled;
#ifdef BT_MODULE_ON
Ble ble;
#endif
ESP32Encoder enc;

void setup();
void setAttRegister(char value);
void shortBeep();
void longBeep();
void saveSettings();
void applyChangesAndRender();

void setup() {
  // put your setup code here, to run once: 
  pinMode(SI_3V_PIN, OUTPUT);
  pinMode(LE_3V_PIN, OUTPUT);
  pinMode(SH_CLK_3V_PIN, OUTPUT);
  pinMode(RESET_3V_PIN, OUTPUT);  
  digitalWrite(RESET_3V_PIN,0); //сброс настроек микросхемы аттенюатора
  delay(1);
  digitalWrite(RESET_3V_PIN,1);
  setAttRegister(0);//установка аттенюатора на максимум при загрузке (на всякий случай, до выхода на рабочий режим и загрузки настроек, чтобы ничего не сгорело в сети);  
  Serial.begin(115200); //debug
  oled.begin(); 
  pinMode(LED_PIN,OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  pinMode(ENC_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  enc.attachFullQuad(ENC_A_PIN, ENC_B_PIN);
  EEPROM.begin(1);//Инициализация EEPROM с размером 1 байт (эмуляция)
  enc_pos = EEPROM.read(0);   // Чтение данных из 0-го адреса
  prev_enc_pos=enc_pos; //выравнивание, чтобы не было факта поворота
  enc.setCount(-1*enc_pos*2); //выравнивание переменной в объекте энкодера
  applyChangesAndRender();
  #ifdef BT_MODULE_ON
  ble.begin();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:    
  enc_pos=-1*floor(enc.getCount()/2);//считывание переменной объекта энкодера
  
  if(enc_pos<0){  //если закрутили за ноль
    enc.setCount(0);
    enc_pos=0;
  }
  
  if(enc_pos>(sizeof(attValue)/sizeof(attValue[0])-1)){ //если перекрутили за максимум        
    enc.setCount(-1*int(((sizeof(attValue)/sizeof(attValue[0]))-1)*2));    
    enc_pos=(sizeof(attValue)/sizeof(attValue[0]))-1;    
  }
  
  if(enc_pos!=prev_enc_pos){ //если был поворот энкодера
    applyChangesAndRender();
    shortBeep();
  }

  if(digitalRead(ENC_BTN_PIN)==0){ //если нажали кнопку энкодера - записать данные во флэш-память        
    if(button_released_flag==1){ //если до этого кнопка была отжата
      button_released_flag=0; //сброс флага, что кнопка была отжата
      saveSettings();      
    }      
  }else{
    button_released_flag=1;//установка флага, что кнопка отжата
  }  
  
  if(millis()>prevMillis+updateRegisterInterval){ //периодическое обновление
    applyChangesAndRender();
    prevMillis=millis();
    Serial.println("updating");
  }

  #ifdef BT_MODULE_ON
  if(ble.recvd()!=""){  //если с BT пришла команда
    delay(100);
    String command_from_ble="";
    command_from_ble=ble.recvd();
    ble.clr();    
    if(command_from_ble.startsWith("s=")){
      command_from_ble.replace("s=", "");
      enc_pos=(signed char)command_from_ble.toInt();
      applyChangesAndRender();
      shortBeep();
    }    
    if(command_from_ble.startsWith("save")){            
      saveSettings();
    } 
    if(command_from_ble.startsWith("request")){            
      ble.send("l="+String(attValue[enc_pos].value_in_db_to_display));
    } 
  }
  #endif
  
  
}

void shortBeep(){
  #ifdef BUZZER_MODE_ON
  for(int i=0;i<5;i++){    
    digitalWrite(BUZZER_PIN,1);
    delay(1);
    digitalWrite(BUZZER_PIN,0);    
    delay(1);
  }
  #endif
}

void longBeep(){
  #ifdef BUZZER_MODE_ON
  for(int i=0;i<500;i++){    
    digitalWrite(BUZZER_PIN,1);
    delay(1);
    digitalWrite(BUZZER_PIN,0);    
    delay(1);
  }
  #endif
}

void setAttRegister(char value){
  digitalWrite(LE_3V_PIN,0);
  delay(1);
  shiftOut(SI_3V_PIN, SH_CLK_3V_PIN, MSBFIRST, value);
  delay(1);
  digitalWrite(LE_3V_PIN,1);//latch enable
  delay(1);
  digitalWrite(LE_3V_PIN,0);
}

void saveSettings(){
  EEPROM.write(0, enc_pos);  // Запись данных
  EEPROM.commit();   // Сохранение изменений    
  longBeep();    
}

void applyChangesAndRender(){ //применение настроек и прорисовка на дисплей
  setAttRegister(attValue[enc_pos].registerValue);  
  oled.print(0,"АТТЕНЮАТОР");  
  oled.print(1,String(attValue[enc_pos].value_in_db_to_display)+" дБ");  
  oled.print(2,"НАЖМИ ДЛЯ");  
  oled.print(3,"СОХР.НАСТРОЕК");  
  oled.update();      
  prev_enc_pos=enc_pos;
  enc.setCount(-1*enc_pos*2);
  #ifdef BT_MODULE_ON
  ble.send("l="+String(attValue[enc_pos].value_in_db_to_display));    
  #endif
}
