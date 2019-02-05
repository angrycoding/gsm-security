#include "SIM800.h"

boolean isEnabled = false;
char* controlMSISDN[] = {"+79260617034", "+79190148644"}; 
    

void setup() {
	Serial.begin(9600);
	SIM800::init(9600, 12, 14);
}

boolean isAllowedMSISDN(String msisdn) {
  uint8_t size = sizeof(controlMSISDN) / sizeof(controlMSISDN[0]);
  for (uint8_t c = 0; c < size; c++) {
    if (msisdn.equals(controlMSISDN[c])) {
      return true;
    }
  }
  return false;
}


void loop() {
	switch (SIM800::update()) {

		case SIM800::CALL:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
  		if (isEnabled = !isEnabled) {
				SIM800::sendSMS(SIM800::msisdn, "включено");
			} else {
				SIM800::sendSMS(SIM800::msisdn, "выключено");
			}
			break;

		case SIM800::SMS:
      if (!isAllowedMSISDN(SIM800::msisdn)) break;
      Serial.print("SMS = ");
      Serial.print(SIM800::msisdn);
      Serial.print(" : ");
      Serial.println(SIM800::text);
			break;

	}
}
