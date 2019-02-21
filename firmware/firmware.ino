#define powerPin 2   
#define rxPin 5
#define txPin 4
#define doorPin 6           
#define windowPin 7         
#define moovePin 8          
  

#include "SIM800.h"

boolean isSecurityEnabled = false;
boolean warning = false;;  
boolean door = false;             
boolean window = false;           
boolean moove= false;        
float roomHumidity = 56.3;               
float roomTemperature = 18.5;            
float outTemperature = -4.5;            
long lastTime=0;        
String doorMsg = "открыта дверь ";
String windowMsg = "открыто окно ";
String mooveMsg = "обнаружено движение ";
String warnSMS = "";
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

void sendingStatus(String responseMSISDN) {

 String security =""; 
 if (isSecurityEnabled == true) security ="Security -On";
 if (isSecurityEnabled == false) security ="Security -Off";
 
 String power =""; 
 if (digitalRead(powerPin) == HIGH) power ="220v-Ok";
 if (digitalRead(powerPin) == LOW) power ="220v-Low"; 
 
 // для справки: СМС всещвет 160 символов на латиннице и 70 на киррилице

 
 String statusSMS = "Security -" + String(security)+ " InsideTemp-";// +  String(roomTemperature) + "°C"+ " InsideHum-" + String(roomHumidity)+"%"; 
        //statusSMS += " OutTemp-" + String(outTemperature)+"°C" + String(power);                              
        //statusSMS += " door-" + String(door) + " window-" + String(window) + " moowe-" + String(moove); 
  // statusSMS - 87 символов на латиннице
SIM800::sendSMS(responseMSISDN,statusSMS); // текст СМС}
}
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
void warnSendSMS(String responseMSISDN) {
   SIM800::sendSMS(responseMSISDN, "тревога");
}
void checkIntrusion() {
  if (!isSecurityEnabled) return;
  if (digitalRead(7) == HIGH) {
    warnSendSMS(SIM800::msisdn);
  }
}

void setup() {
    pinMode(16, OUTPUT);
  Serial.begin(9600);
  SIM800::init(9600, rxPin, txPin);
}

void loop() {

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

  checkIntrusion();
     if (isSecurityEnabled) digitalWrite (13, HIGH);
   else digitalWrite (13, LOW);
}
