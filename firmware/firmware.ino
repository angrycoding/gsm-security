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
    Serial.print(msisdn);
    Serial.print(" compare with ");
    Serial.print(controlMSISDN[c]);
    Serial.print(" = ");
    Serial.println(msisdn.equals(controlMSISDN[c]));
  }
}


void loop() {
	switch (SIM800::update()) {

    if (!isAllowedMSISDN(SIM800::msisdn)) break;

		case SIM800::CALL:
  		if (isEnabled = !isEnabled) {
				SIM800::sendSMS(SIM800::msisdn, "включено");
			} else {
				SIM800::sendSMS(SIM800::msisdn, "выключено");
			}
			break;

		case SIM800::SMS:
      Serial.print("SMS = ");
      Serial.print(SIM800::msisdn);
      Serial.print(" : ");
      Serial.println(SIM800::text);
			break;

	}
}
