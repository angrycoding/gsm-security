#ifndef SIM800_h
#define SIM800_h

#include <SoftwareSerial.h>
// perhaps that has to be removed in nodemcu
#include <Arduino.h>

namespace SIM800_private {

	SoftwareSerial *sim800;

	String waitResponse() {
		String response = "";
		long timeout = millis() + 10000; 
		while (!sim800->available() && millis() < timeout);
		if (sim800->available()) {
			response = sim800->readString();
			response.trim();
			response.toLowerCase();
			response.replace("\n", "");
			response.replace("\r", "");
			response.replace(" ", "");
		}
		return response;
	}

	String sendATCommand(String cmd) {
		Serial.print("REQUEST = ");
		Serial.println(cmd);
		sim800->println(cmd);
		String response = waitResponse();
		Serial.print("RESPONSE = ");
		Serial.println(response);
		return response;
	}

	unsigned int getCharSize(unsigned char b) {
		if (b >= 128) for (int i = 1; i <= 7; i++) {
			if (((b << i) & 0xFF) >> 7 == 0) {
				return i;
			}
		}
		return 1;
	}

	unsigned int symbolToUInt(const String& bytes) {
		unsigned int charSize = bytes.length();
		unsigned int result = 0;
		if (charSize == 1) return bytes[0];
		unsigned char actualByte = bytes[0];
		result = actualByte & (0xFF >> (charSize + 1));
		result = result << (6 * (charSize - 1));
		for (int i = 1; i < charSize; i++) {
			actualByte = bytes[i];
			if ((actualByte >> 6) != 2) return 0;
			result |= ((actualByte & 0x3F) << (6 * (charSize - 1 - i)));
		}
		return result;
	}

	String byteToHexString(uint8_t i) {
		String hex = String(i, HEX);
		if (hex.length() == 1) hex = "0" + hex;
		hex.toUpperCase();
		return hex;
	}

	String StringToUCS2(String s) {
		String result = "";
		for (int k = 0; k < s.length(); k++) {
			unsigned int charSize = getCharSize((uint8_t)s[k]);
			char symbolBytes[charSize + 1];
			for (int i = 0; i < charSize; i++) symbolBytes[i] = s[k + i];
			symbolBytes[charSize] = '\0';
			unsigned int charCode = symbolToUInt(symbolBytes);
			if (charCode > 0)  result += byteToHexString((charCode & 0xFF00) >> 8) + byteToHexString(charCode & 0xFF);
			k += charSize - 1;
		}
		return result;
	}

	String getDAfield(String *phone, bool fullnum) {
		String result = "";
		for (int i = 0; i <= (*phone).length(); i++) {
			if (isDigit((*phone)[i])) {
				result += (*phone)[i];
			}
		}
		uint8_t phonelen = result.length();
		if (phonelen % 2 != 0) result += "F";
		for (int i = 0; i < result.length(); i += 2) {
			char symbol = result[i + 1];
			result = result.substring(0, i + 1) + result.substring(i + 2);
			result = result.substring(0, i) + (String)symbol + result.substring(i);
		}
		result = fullnum ? "91" + result : "81" + result;
		result = byteToHexString(phonelen) + result;
		return result;
	}

	void getPDUPack(String *phone, String *message, String *result, int *PDUlen) {
		*result += "01";
		*result += "00";
		*result += getDAfield(phone, true);
		*result += "00";
		*result += "08";
		String msg = StringToUCS2(*message);
		*result += byteToHexString(msg.length() / 2);
		*result += msg;
		*PDUlen = (*result).length() / 2;
		*result = "00" + *result;
	}

}


namespace SIM800 {

	const uint8_t CALL = 1;
	const uint8_t SMS = 2;

	String msisdn = "";
	String text = "";

	// процедура инициализации, вызывается в setup()
	// принимает скорость обмена и номера пинов
	void init(uint32_t speed, uint8_t PIN_RX, uint8_t PIN_TX) {
		using namespace SIM800_private;
		sim800 = new SoftwareSerial(PIN_RX, PIN_TX);
		sim800->begin(speed);
		sendATCommand("ATE0");
		sendATCommand("AT");
		sendATCommand("AT+CMGF=1");
		sendATCommand("AT+CNMI=1, 2, 0, 0, 0");
		sendATCommand("AT+CLIP=1");
	}

	void sendSMS(String phone, String message) {
		using namespace SIM800_private;
		String *ptrphone = &phone;
		String *ptrmessage = &message;
		String PDUPack;
		String *ptrPDUPack = &PDUPack;
		int PDUlen = 0;
		int *ptrPDUlen = &PDUlen;
		getPDUPack(ptrphone, ptrmessage, ptrPDUPack, ptrPDUlen); 
		sendATCommand("AT+CMGF=0");
		sendATCommand("AT+CMGS=" + (String)PDUlen);
		sendATCommand(PDUPack + (String)((char)26));
	}

	uint8_t update() {
		using namespace SIM800_private;
		if (!sim800->available()) return 0;
		
		uint8_t status = 0;
		String request = sim800->readString();
		request.trim();
		request.toLowerCase();
		request.replace("\n", "");
		request.replace("\r", "");
		request.replace(" ", "");

		if (request.startsWith("ring")) {
			sendATCommand("AT+CHUP");
			request = request.substring(4);
			if (request.startsWith("+clip:\"")) {
				msisdn = request.substring(7, request.indexOf('"', 7));
				if (msisdn.length() >= 12) status = CALL;
			}
		}

		else if (request.startsWith("+cmt:\"")) {
			msisdn = request.substring(6, request.indexOf('"', 6));
			text = request.substring(request.lastIndexOf('"') + 1);
			if (msisdn.length() >  12 && text.length() > 0) {
				status = SMS;
			}
		}

		return status;
	}

}

#endif