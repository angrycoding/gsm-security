#define powerPin 12   
#define rxPin 3
#define txPin 2
#define doorPin 9           
#define windowPin 8         
#define moovePin 7          
#define DHTPin 11 
#define ds18b20Pin 10

#include "SIM800.h"
#include <DHT.h>   
#include <OneWire.h>     
#include <DallasTemperature.h>


DHT dht(DHTPin, DHT11);
OneWire oneWire(ds18b20Pin); 
DallasTemperature sensors(&oneWire);

bool isSecurityEnabled =0;
int roomHumidity;               
int roomTemperature;   
int outTemperature;       
long lastTime=0;  
boolean warning = false;;  
boolean door = false;             
boolean window = false;           
boolean moove= false;    
boolean warn= false;   
boolean Power=false;
String warnSMS = "";
String power ="220-Ok"; 
char* controlMSISDN[] = {"+79260617034", "+79190148644"}; 


boolean isAllowedMSISDN(String msisdn) {
  uint8_t size = sizeof(controlMSISDN) / sizeof(controlMSISDN[0]);
  for (uint8_t c = 0; c < size; c++) {
    if (msisdn.equals(controlMSISDN[c])) {
      return true;
    }
  }
  return false;
}
//-------------ОТПРАВКА СТАТУСА------------------------------------------------
void sendingStatus(String responseMSISDN) {
 
 String security =""; 
 if (isSecurityEnabled == true) security ="Oxp=1";
 if (isSecurityEnabled == false) security ="Oxp=0";

sensors.requestTemperatures();
outTemperature =sensors.getTempCByIndex(0);
 
roomHumidity   = dht.readHumidity();    
roomTemperature = dht.readTemperature(); 
 
String Temp = "in=" + String(roomTemperature);      
String Hum = "hum=" + String(roomHumidity);    
String outTemp = "out=" + String(outTemperature);  
String statusSMS1 = security+"\n"+power+"\n"+Hum + "\n" +Temp+"\n"+ outTemp;
String statusSMS2 = String (statusSMS2)+"\nd="+door+"\nw="+window+"\nm="+moove;


SIM800::sendSMS(responseMSISDN,statusSMS1); 
SIM800::sendSMS(responseMSISDN,statusSMS2); 
}
//------------ОТПРАВКА ТРЕВОГИ---------------------
void sendWarnSMS(String responseMSISDN) {
  SIM800::sendSMS(responseMSISDN,warnSMS); 
}
//------------ОТПРАКА ОТВЕТНЫХ СМС О СОСТОЯНИИ ОХРАНЫ--------------------------
void enableSecurity(String responseMSISDN) {
  if (isSecurityEnabled) {
    SIM800::sendSMS(responseMSISDN, "охрана уже включена");
    return;
  }
  isSecurityEnabled = true;
  SIM800::sendSMS(responseMSISDN, "постановка на охрану");
}

void disableSecurity(String responseMSISDN) {
  if (!isSecurityEnabled) {
    SIM800::sendSMS(responseMSISDN, "охрана уже отключена");
    return;
  }
  isSecurityEnabled = false;
  SIM800::sendSMS(responseMSISDN, "снятие с охраны");
}
// -----------------ПРОВЕРКА ДАТЧИКОВ-------------------------------
void checkSensors() {
   if (digitalRead(doorPin) == HIGH)  door = true,   warn = true;
   if (digitalRead(windowPin) == HIGH)window = true, warn = true;  
   if (digitalRead(moovePin) == HIGH) moove = true,  warn = true;  
   if (digitalRead(powerPin) == HIGH) Power=true;
   if (digitalRead(powerPin) == LOW)  Power=false;
  }

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  SIM800::init(9600, rxPin, txPin);
  sensors.begin();
}

void loop() {
checkSensors(); 
  switch (SIM800::update()) {

    case SIM800::CALL:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
      if (isSecurityEnabled) {
        disableSecurity(SIM800::msisdn);
        sendingStatus(SIM800::msisdn);
      } else {
        enableSecurity(SIM800::msisdn);
        sendingStatus(SIM800::msisdn);
      }
      break;

    case SIM800::SMS:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
      if (SIM800::text.equals("1")) {
        enableSecurity(SIM800::msisdn);
      } else if (SIM800::text.equals("0")) {
        disableSecurity(SIM800::msisdn);
      } else if (SIM800::text.equals("2")) {  // если пришел запрос состояния
         sendingStatus(SIM800::msisdn);  }           
      break;

  }

   if (isSecurityEnabled) digitalWrite (13, HIGH);
   else digitalWrite (13, LOW);

if (!isSecurityEnabled) return;  // далее код выполняется только если isSecurityEnabled=1

   if ((millis () -lastTime)>2000)
   {
       lastTime=millis();
       if(door) {warnSMS = String(warnSMS)+"открыта дверь\n"; door=false;}
       if(window){warnSMS = String(warnSMS)+"открыто окно\n"; window=false;}
       if(moove) {warnSMS = String(warnSMS)+"движение\n"; moove=false;}
       if(Power==false && power=="220-Ok") warnSMS = String(warnSMS)+"отключено питание\n", power="220--"; 
       if(Power== true && power=="220--")  warnSMS = String(warnSMS)+"подключено питание\n", power="220-Ok"; 
       if(warnSMS!="") sendWarnSMS(SIM800::msisdn),warnSMS="";
    }
   
}
