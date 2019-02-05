#include "SIM800.h"


boolean isEnabled = false;


void setup() {
	Serial.begin(9600);
	SIM800::init(9600, 12, 14);
}


void loop() {
	switch (SIM800::update()) {

		case SIM800::CALL:
			//
			if (isEnabled = !isEnabled) {
				SIM800::sendSMS(SIM800::msisdn, "включено");
			} else {
				SIM800::sendSMS(SIM800::msisdn, "выключено");
			}
			break;

		case SIM800::SMS:
			//SIM800::msisdn
			//SIM800::text
			break;

	}
}
