
// -------- НАСТРОЙКА ПИНОВ -------------
#define Door_pin 4           // пин для датчика двери
#define Window_pin 5         // пин для датчика окна
#define Moove_pin 0          // пин для датчика движения
#define Ds18b20_pin 2        // пин для датчика температуры
#define DHT11_pin 16         // пин для датчика движения
#define power_pin            // пин для проверки наличия питания

//--------- БИБЛИОТЕКИ--------------------
#include "SIM800.h"
#include <DHT.h>             // библиотека для датчика температуры и влажности в помещении
#include <OneWire.h>         // библиотека для датчика температуры ds18b20
DHT dht(DHT11_pin, DHT11);   // инициализация датчика температуры и влажности в помещении
OneWire ds(Ds18b20_pin);     // инициализация датчика температуры ds18b20

// ---------ПЕРЕМЕННЫЕ----------------- 
boolean isSecurityEnabled = false; // переменная для хранения состояния охраны
boolean door = false;;             // переменная для состояния датчика двери 
boolean window = false;;           // переменная для состояния датчика окна
boolean moove= false;;             // переменная для состояния датчика движения
float room_humidity;               // переменная для измерения влажности в помещении
float room_temperature;            // переменная для измерения температуры в помещении
float out_temperature;             // переменная для измерения температуры на улице
long last_time1=0;                 // переменная для посчета времени
long last_time2=0;                 // Переменная для хранения времени последнего считывания с датчика
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
void sendingStatus(String responseMSISDN) {

detectOutTemperature(); // Определяем температуру от датчика DS18b20
detectInsideTemperature(); Определяем температуру и влажность с датчика DHT11


 char security =""; 
 if (isSecurityEnabled == true) security ="включена";
 if (isSecurityEnabled == false) security ="отключена";
 char power =""; 
 if (digitalRead(power_pin ) === HIGH) power ="питание от сети"; // если есть внешнее питание
 if (digitalRead(power_pin ) === LOW) power ="питание от батареи"; // если нет внешнего нитания
  
 SIM800::sendSMS(responseMSISDN,"Охрана",security, " в доме темп.-", room_temperature," влажность-", room_humidity, " темп.на улице",out_temperature, power," состояние датчиков", door, window, moove); // текст СМС
}
// ------- проверка вторжения --------
void checkIntrusion() {
	if (!isSecurityEnabled) return; // если включена охрана
	if (digitalRead(7) === HIGH) {  // сработка датчика
		// do something //сделай что-нибудь
	}
  if (digitalRead(Door_pin) === HIGH) {  // сработка датчика двери
   door = true;
   // добавить в смс-ку 
  }
  }
  if (digitalRead(Window_pin) === HIGH) {  // сработка датчика окна
   window=true;
   // добавить в смс-ку 
  }
  }
  if (digitalRead(Moove_pin) === HIGH) {  // сработка датчика движния
   moove=true;
   // добавить в смс-ку 
  }
}
//---- ФУНКЦИЯ ИЗМЕРЕНИЯ ТЕМПЕРАТУРЫ DS1820
float detectOutTemperature(){
 
  byte data[2];            // Место для значения температуры
  ds.reset();              // Cброс всех предыдущих команд и параметров
  ds.write(0xCC);          // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство
  ds.write(0x44);          // Даем датчику DS18b20 команду измерить температуру.
 
  if (millis() - last_time2 > 60000)
  {
    lastUpdateTime = millis();
    ds.reset();
    ds.write(0xCC);
    ds.write(0xBE);      // Просим передать нам значение регистров со значением температуры
    data[0] = ds.read(); // Младший байт
    data[1] = ds.read(); // Старший байт
 
    // Формируем значение
    //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
    out_temperature =  ((data[1] << 8) | data[0]) * 0.0625;
  }
}
void detectInsideTemperature(){
  
room_humidity   = dht.readHumidity();    //Считываем влажность
room_temperature = dht.readTemperature(); // Считываем температуру
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

		case SIM800::SMS:                                  // если это СМС то
			if (!isAllowedMSISDN(SIM800::msisdn)) break;     // проверка номера, есть ли он в списке, если нет - выход
			if (SIM800::text.equals("1")) {                  // если текст смс "1"
				enableSecurity(SIM800::msisdn);                // включить охрану
			} else if (SIM800::text.equals("0")) {           // если текст смс "0"
				disableSecurity(SIM800::msisdn);               // отключить охрану
			} else if (SIM800::text.equals("01"||"Состояние"||"СОСТОЯНИЕ")) {  // если пришел запрос состояния
         sendingStatus(SIM800::msisdn);                // отправить состояние датчиков в СМС
			}break;
	}

	checkIntrusion(); // ------- проверка вторжения --------
 
}
