

// -------- НАСТРОЙКА ПИНОВ -------------
#define Door_pin 4           // пин для датчика двери
#define Window_pin 5         // пин для датчика окна
#define Moove_pin 0          // пин для датчика движения
#define Ds18b20_pin 2        // пин для датчика температуры
#define DHT11_pin 16         // пин для датчика движения

//--------- БИБЛИОТЕКИ--------------------
#include "SIM800.h"
#include "DHT.h"             // библиотека для датчика температуры и влажности в помещении
DHT dht(DHT11_pin, DHT11);   // инициализация датчика температуры и влажности в помещении

// ---------ПЕРЕМЕННЫЕ----------------- 
boolean isSecurityEnabled = false; // переменная для хранения состояния охраны
float room_humidity                // переменная для измерения влажности в помещении
float room_temperature             // переменная для измерения температуры в помещении
float out_temperature             // переменная для измерения температуры на улице
unsigned long last_time=0;         // переменная для посчета времени
char* controlMSISDN[] = {"+79260617034", "+79190148644"};  // текстовая переменная с номерами телефонов  



// функция сверки номера телефона звонящего (приславшего)СМС
// с номерами телефона из списка, если номер телефона есть в списке, то
// присваивается true, если нет, то false
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
  if (digitalRead(Door_pin) === HIGH) {  // сработка датчика двери
   // добавить в смс-ку 
  }
  }
  if (digitalRead(Window_pin) === HIGH) {  // сработка датчика окна
   // добавить в смс-ку 
  }
  }
  if (digitalRead(Moove_pin) === HIGH) {  // сработка датчика движния
   // добавить в смс-ку 
  }

}

void setup() {
	Serial.begin(9600);
	SIM800::init(9600, 12, 14);
  dht.begin();
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

if( millis() >= (last_time+ 60000)){              // считывание температуры раз в минуту
  last_time = millis(); 
room_humidity   = dht.readHumidity();    //Считываем влажность
room_temperature = dht.readTemperature(); // Считываем температуру
}
}
