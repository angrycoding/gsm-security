#include "SIM800.h"

boolean isSecurityEnabled = false;
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
void warnSMS(String responseMSISDN) {
   SIM800::sendSMS(responseMSISDN, "тревога");
}
void checkIntrusion() {
  if (!isSecurityEnabled) return;
  if (digitalRead(7) == HIGH) {
    warnSMS(SIM800::msisdn);
  }
}

void setup() {
    pinMode(16, OUTPUT);
  Serial.begin(9600);
  SIM800::init(9600, 5, 4);
}

void loop() {

  switch (SIM800::update()) {

    case SIM800::CALL:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
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
      }
      break;

  }

  checkIntrusion();
     if (isSecurityEnabled) digitalWrite (13, HIGH);
   else digitalWrite (13, LOW);
}
