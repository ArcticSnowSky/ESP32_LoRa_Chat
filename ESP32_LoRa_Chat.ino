#include <heltec.h>

/*
 * ESP32 LoRa test code
 * 
 * Chatting through LORA
 *
 * by Thomas Mailänder <mailaender.t@gmail.com>
 * 19.01.2020
*/

#include "Arduino.h"
#include "heltec.h"
#include "images.h"
#include "chat.h"
#include "utils.h"


#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

//#define DISPLAY_CHAR_WIDTH	80


Chat chat;
bool deepsleepflag=false;

void setup()
{
	Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);
  
	logo();
	delay(500);

	Serial.flush();
	Heltec.display->setFont(ArialMT_Plain_24);
	Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
	Heltec.display->drawString(64,14, "Chat bereit");
	Heltec.display->setFont(ArialMT_Plain_16);
	Heltec.display->drawString(64, 44, "Serialport 115200");
	Heltec.display->display();
	delay(1000);
	Heltec.display->clear();
	chat.init();

	Serial.println("Chat bereit\nEingaben über serialport möglich\nSchreibe /help für Hilfsbefehle");

	attachInterrupt(KEY_BUILTIN, interrupt_GPIO0, FALLING);
	LoRa.onReceive(onReceive);
	LoRa.receive();
	Serial.setTimeout(0);
}

void loop()
{
	if(deepsleepflag)
	{
		Serial.println("Entering deepSleep");
		LoRa.end();
		LoRa.sleep();
		delay(100);
		pinMode(4,INPUT);
		pinMode(5,INPUT);
		pinMode(14,INPUT);
		pinMode(15,INPUT);
		pinMode(16,INPUT);
		pinMode(17,INPUT);
		pinMode(18,INPUT);
		pinMode(19,INPUT);
		pinMode(26,INPUT);
		pinMode(27,INPUT);
		digitalWrite(Vext,HIGH);
		delay(2);
		esp_deep_sleep_start();
	}

	if(Serial.available())
	{
		String msg = readStringLn();
		if (msg != NULL)
		{
			send(msg);
			
			checkCommands(msg);
		}
	}
	chat.process();
}


void checkCommands(String msg)
{
	if (msg.startsWith("/")) {
		msg = msg.substring(1);
		msg.toLowerCase();
		if (msg == "help") {
			Serial.println("TestBefehle");
			Serial.println("");
			Serial.println("Befehle müssen /cmd [true|false|integer] geschrieben werden");
			Serial.println("  deepSleepFlag : DeepSleep, Wake unbekannt");
			Serial.println("  sleeptimer    : Schläft für x|20 sekunden");
			Serial.println("  sleepkey      : Bei Tastendruck");
			Serial.println("  gpio25        : Schaltet led");
			Serial.println("  help          : Diese hilfe");
			return;
		}
		else if (msg.startsWith("sleeptimer")) {
			char wakeup_time_sec_txt[8];
			msg.substring(10).toCharArray(wakeup_time_sec_txt, sizeof(wakeup_time_sec_txt));
			int wakeup_time_sec = atoi(wakeup_time_sec_txt);
			if (wakeup_time_sec == 0)
				wakeup_time_sec = 20;
			Serial.printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
			delay(15);
			esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
			esp_deep_sleep_start();
			return;
		}
		else if (msg == "sleepkey") {
			Serial.println("Enabling ext wakeup");
			delay(15);
			esp_sleep_enable_ext1_wakeup(1ULL << KEY_BUILTIN, ESP_EXT1_WAKEUP_ALL_LOW);
			esp_deep_sleep_start();
		}
		switch(txtEndsWithBool(msg, "deepsleepflag")) {
			case ENDS_WITH_TRUE:
				deepsleepflag = true;
				return;
			case ENDS_WITH_FALSE:
				deepsleepflag = false;
				return;
		}
		switch(txtEndsWithBool(msg, "gpio25")) {
			case ENDS_WITH_TRUE:
				digitalWrite(25,HIGH);
				return;
			case ENDS_WITH_FALSE:
				digitalWrite(25,LOW);
				return;
		}
	}
}

void logo(){
	Heltec.display -> clear();
	Heltec.display -> drawXbm(0,0,logo_ss_width,logo_ss_height,(const unsigned char *)logo_ss_bits);
	Heltec.display -> display();
	Heltec.display -> clear();
}


void interrupt_GPIO0()
{
	delay(10);
	if(digitalRead(KEY_BUILTIN)==LOW)
	{
		Serial.print("interrupt_GPIO0->digitalRead(KEY_BUILTIN)==LOW");
		if(digitalRead(LED)==HIGH) {
			Serial.print("->digitalRead(LED)==HIGH");
			deepsleepflag=true;
		}
		Serial.println();
	} else {
		Serial.println("\0");
		Serial.flush();
		//delay(200);
		Serial.println("interrupt_GPIO0 (Serial Connection?)");
		Serial.println("Schreibe /help für Hilfe");
	}
}

void onReceive(int packetSize)//LoRa receiver interrupt service
{
	String rcvdMsg = "";
	while (LoRa.available())
		rcvdMsg += (char) LoRa.read();
	rcvdMsg += "\0";
	chat.topInfo = "RSSI " + String(LoRa.packetRssi(), DEC) + "  SNR "+ String(LoRa.packetSnr(), DEC);
	chat.received(rcvdMsg);
}

void send(String msg)
{
	LoRa.beginPacket();
	LoRa.print(msg);
	LoRa.endPacket();
	LoRa.receive();
	chat.sent(msg);
}
