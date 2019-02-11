
// -------- НАСТРОЙКА ПИНОВ -------------
#define doorPin 4           
#define windowPin 5         
#define moovePin 0          
#define ds18b20Pin 2       
#define DHT11Pin 16        
#define powerPin            

//--------- БИБЛИОТЕКИ--------------------
#include "SIM800.h"
#include <DHT.h>             
#include <OneWire.h>         
DHT dht(DHT11Pin, DHT11);   
OneWire ds(ds18b20Pin);    

// ---------ПЕРЕМЕННЫЕ----------------- 
boolean isSecurityEnabled = false; 
boolean door = false;;             
boolean window = false;;           
boolean moove= false;;             
float roomHumidity;               
float roomTemperature;            
float outTemperature;            
long lastTime=0;                 
char* controlMSISDN[] = {"+79260617034", "+79190148644"};  


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
		SIM800::sendSMS(responseMSISDN, "охрана уже включена"); 
		return;
	}
	isSecurityEnabled = true;      // охрана включена
	SIM800::sendSMS(responseMSISDN, "постановка на охрану"); 
}
//--------отключение охраны-------
void disableSecurity(String responseMSISDN) {
	if (!isSecurityEnabled) {
		SIM800::sendSMS(responseMSISDN, "охрана уже отключена"); 
		return;
	}
	isSecurityEnabled = false;      // охрана отключена
	SIM800::sendSMS(responseMSISDN, "снятие с охраны"); 
}
void sendingStatus(String responseMSISDN) {

detectOutTemperature();
detectInsideTemperature();


 char security =""; 
 if (isSecurityEnabled == true) security ="включена";
 if (isSecurityEnabled == false) security ="отключена";
 char power =""; 
 if (digitalRead(powerPin) === HIGH) power ="питание от сети";
 if (digitalRead(powerPin) === LOW) power ="питание от батареи"; 
  
 SIM800::sendSMS(responseMSISDN,"Охрана",security, " в доме темп.-", roomTemperature," влажность-", roomHumidity, " темп.на улице",outTemperature, power," состояние датчиков", door, window, moove); // текст СМС
}
// ------- проверка вторжения --------
void checkIntrusion() {
	if (!isSecurityEnabled) return; // если включена охрана
	if (digitalRead(7) === HIGH) { 
	
	}
  if (digitalRead(doorPin) === HIGH) { 
   door = true;
   // добавить в смс-ку 
  }
  }
  if (digitalRead(windowPin) === HIGH) {  
   window=true;
   // добавить в смс-ку 
  }
  }
  if (digitalRead(moovePin) === HIGH) {  
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
 
  if (millis() - lastTime > 60000)
  {
    lastUpdateTime = millis();
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
  
roomHumidity   = dht.readHumidity();    //Считываем влажность
roomTemperature = dht.readTemperature(); // Считываем температуру
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
			} else if (SIM800::text.equals("01")||SIM800::text.equals("Состояние")||SIM800::text.equals("СОСТОЯНИЕ")) {  // если пришел запрос состояния
         sendingStatus(SIM800::msisdn);                // отправить состояние датчиков в СМС
			}break;
	}

	checkIntrusion(); // ------- проверка вторжения --------
 
}
