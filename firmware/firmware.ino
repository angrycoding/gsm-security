#include "SIM800.h"

boolean isSecurityEnabled = false;                         // переменная для хранения состояния охраны
char* controlMSISDN[] = {"+79260617034", "+79190148644"};  // текстовая переменная с номерами телефонов  


// функция сверки номера телефона звонящего (приславшего)СМС
// с номерами телефона из списка, если номер телефона есть в списке, то
// присваивается true, если нет, то false
boolean isAllowedMSISDN(String msisdn) {    
	uint8_t size = sizeof(controlMSISDN) / sizeof(controlMSISDN[0]);
	for (uint8_t c = 0; c < size; c++) {
		if (msisdn.equals(controlMSISDN[c])) {
			return true;  // не совсем понимаю куда вернуть правду...
		}
	}
	return false; // ..и куда ложь
}
//-------- включение охраны------
void enableSecurity(String responseMSISDN) {
	if (isSecurityEnabled) {
		SIM800::sendSMS(responseMSISDN, "охрана уже включена"); // ответная СМС
		return;
	}
	isSecurityEnabled = true;      // охрана включена
	SIM800::sendSMS(responseMSISDN, "постановка на охрану"); // ответная СМС
}
//--------отключение охраны-------
void disableSecurity(String responseMSISDN) {
	if (!isSecurityEnabled) {
		SIM800::sendSMS(responseMSISDN, "охрана уже отключена"); // ответная СМС
		return;
	}
	isSecurityEnabled = false;      // охрана отключена
	SIM800::sendSMS(responseMSISDN, "снятие с охраны"); // ответная СМС
}
// ------- проверка вторжения --------
void checkIntrusion() {
	if (!isSecurityEnabled) return; // если включена охрана
	if (digitalRead(7) === HIGH) {  // сработка датчика
		// do something //сделай что-нибудь
	}
}

void setup() {
	Serial.begin(9600);
	SIM800::init(9600, 12, 14);
}

void loop() {

	switch (SIM800::update()) {                      // если от модуля что-то пришло

		case SIM800::CALL:                             // если это звонок, то
			if (!isAllowedMSISDN(SIM800::msisdn)) break; // проверка номера, есть ли он в списке, если нет - выход
			if (isSecurityEnabled) {
				disableSecurity(SIM800::msisdn);
			} else {
				enableSecurity(SIM800::msisdn);
			}
			break;

		case SIM800::SMS:                                 // если это СМС то
			if (!isAllowedMSISDN(SIM800::msisdn)) break;   // проверка номера, есть ли он в списке, если нет - выход
			if (SIM800::text.equals("1")) {                // если текст смс "1"
				enableSecurity(SIM800::msisdn);              // включить охрану
			} else if (SIM800::text.equals("0")) {         // если текст смс "0"
				disableSecurity(SIM800::msisdn);             // отключить охрану
			}
			break;

	}

	checkIntrusion(); // ------- проверка вторжения --------

}
