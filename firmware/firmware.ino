// -------- НАСТРОЙКА ПИНОВ -------------
#define doorPin 2           
#define windowPin 3         
#define moovePin 6          
#define ds18b20Pin 7       
#define DHT11Pin 8        
#define powerPin 9         

//--------- БИБЛИОТЕКИ--------------------
#include "SIM800.h"
#include <DHT.h>             
#include <OneWire.h>         
DHT dht(DHT11Pin, DHT11);   
OneWire ds(ds18b20Pin);    

// ---------ПЕРЕМЕННЫЕ----------------- 
boolean isSecurityEnabled = false;
boolean warning = false;;  
boolean door = false;             
boolean window = false;           
boolean moove= false;             
float roomHumidity;               
float roomTemperature;            
float outTemperature;            
long lastTime=0;                 
char* controlMSISDN[] = {"+79260617034", "+79190148644"};  
String doorMsg = "открыта дверь ";
String windowMsg = "открыто окно ";
String mooveMsg = "обнаружено движение ";
String warnSMS = "";

// функция сверки номера телефона звонящего (приславшего СМС)
boolean isAllowedMSISDN(String msisdn) {    
  uint8_t size = sizeof(controlMSISDN) / sizeof(controlMSISDN[0]);
  for (uint8_t c = 0; c < size; c++) {
    if (msisdn.equals(controlMSISDN[c])) {
      return true;  
    }
  }
  return false; 
}
//-------- включение охраны------
void enableSecurity(String responseMSISDN) {
  if (isSecurityEnabled) {
    SIM800::sendSMS(responseMSISDN, "охрана уже включена"); 
    return;
  }
  isSecurityEnabled = true;     
  SIM800::sendSMS(responseMSISDN, "постановка на охрану"); 
}
//--------отключение охраны-------
void disableSecurity(String responseMSISDN) {
  if (!isSecurityEnabled) {
    SIM800::sendSMS(responseMSISDN, "охрана уже отключена"); 
    return;
  }
  isSecurityEnabled = false;      
  SIM800::sendSMS(responseMSISDN, "снятие с охраны"); 
}
void sendingStatus(String responseMSISDN) {

detectOutTemperature();
detectInsideTemperature();

 String security =""; 
 if (isSecurityEnabled == true) security ="включена";
 if (isSecurityEnabled == false) security ="отключена";
 
 String power =""; 
 if (digitalRead(powerPin) == HIGH) power ="питание от сети";
 if (digitalRead(powerPin) == LOW) power ="питание от батареи"; 
 
 String statusSMS = "Охрана" + String(security);
        statusSMS += " темп.в доме-" +  String(roomTemperature) + " влажность-" + String(roomHumidity);
        statusSMS += " темп.на улице-" + String(outTemperature) + String(power);
        statusSMS += " состояние датчиков: двери-" + String(door) + " окон-" + String(window) + " движения-" + String(moove);
        
SIM800::sendSMS(responseMSISDN,"Cостояние норм)"); // текст СМС}
}
// ------- проверка вторжения --------
void checkIntrusion() {
  if (!isSecurityEnabled) return; 
  if (digitalRead(7) == HIGH) { 
  warning = true;
  }
  if (digitalRead(doorPin) == HIGH) { 
   door = true;
   warning = true;
  }
  
  if (digitalRead(windowPin) == HIGH) {  
   window=true;
   warning = true;
  }
  
  if (digitalRead(moovePin) == HIGH) {  
   moove=true;
   warning = true;
  }
}
void warnSmsSend(String responseMSISDN) {

if((millis()-lastTime)> 60000){
    lastTime = millis();
    if (door)warnSMS += doorMsg, door = false;
    if(window)warnSMS += windowMsg, window = false;
    if (moove)warnSMS += mooveMsg, moove = false; 
  }
if (warning&&warnSMS!="")SIM800::sendSMS(responseMSISDN,"Тревога:" + String(warnSMS)), warning = false;
}
//---- ФУНКЦИЯ ИЗМЕРЕНИЯ ТЕМПЕРАТУРЫ DS1820
float detectOutTemperature(){
 
  byte data[2];            // Место для значения температуры
  ds.reset();              // Cброс всех предыдущих команд и параметров
  ds.write(0xCC);          // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство
  ds.write(0x44);          // Даем датчику DS18b20 команду измерить температуру.
 
  if (millis() - lastTime > 60000)
  {
    lastTime = millis();
    ds.reset();
    ds.write(0xCC);
    ds.write(0xBE);      // Просим передать нам значение регистров со значением температуры
    data[0] = ds.read(); // Младший байт
    data[1] = ds.read(); // Старший байт
 
    // Формируем значение
    //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
    outTemperature =  ((data[1] << 8) | data[0]) * 0.0625;
  }
}
void detectInsideTemperature(){
  
roomHumidity   = dht.readHumidity();    
roomTemperature = dht.readTemperature(); 
}
  

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  SIM800::init(9600, 5, 4);
  dht.begin();
}

void loop() {

  switch (SIM800::update()) {                      // если от модуля что-то пришло

    case SIM800::CALL:                             
      if (!isAllowedMSISDN(SIM800::msisdn)) break; // проверка номера, есть ли он в списке, если нет - выход
      if (isSecurityEnabled) {
        disableSecurity(SIM800::msisdn);
      } else {
        enableSecurity(SIM800::msisdn);
      }
      break;

    case SIM800::SMS:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
      if (SIM800::text.equals("1")) {
        enableSecurity(SIM800::msisdn);
      } else if (SIM800::text.equals("0")) {
        disableSecurity(SIM800::msisdn);
      } else if (SIM800::text.equals("2")) {  // если пришел запрос состояния
         sendingStatus(SIM800::msisdn);                
      }break;
  }

  checkIntrusion(); 
 // warnSmsSend(SIM800::msisdn);

   if (isSecurityEnabled) digitalWrite (13, HIGH);
   else digitalWrite (13, LOW);
}
