#include <heltec.h>

/*
 * ESP32 LoRa test code
 * 
 * Chatting through LORA
 * 
 * Use SPP Terminal via Bluetooth or usb serial
 *
 * by Thomas Mailänder <mailaender.t@gmail.com>
 * 19.01.2020
*/

#include "Arduino.h"
#include "heltec.h"
#include "images.h"
#include "chat.h"
#include "utils.h"
#include "dualstream.h"
#include "multistream.h"

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

//#define DISPLAY_CHAR_WIDTH	80


Chat chat;
BluetoothSerial SerialBT;
/*Stream* streams[] = {&Serial, &SerialBT};
int STREAMS_LEN (sizeof(streams) / sizeof(Stream*));
static MultiStream ms = MultiStream(streams, STREAMS_LEN);*/
static DualStream dualStream = DualStream(Serial, SerialBT);

bool deepsleepflag = false;	// Entweder per Befehl oder wenn Led Aktiv ist, Taste Drücken


void setup()
{
	Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);
  
	logo();
	SerialBT.begin("ESP32 SS Test"); //Bluetooth device name
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
	chat.init(&dualStream);

	dualStream.println("Chat bereit\nEingaben über serialport möglich\nSchreibe /help für Hilfsbefehle");

	attachInterrupt(KEY_BUILTIN, interrupt_GPIO0, FALLING);
	LoRa.onReceive(onReceive);
	LoRa.receive();
	Serial.setTimeout(0);
}

void loop()
{
	if(deepsleepflag)
	{
		dualStream.println("Entering deepSleep");
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

	if(dualStream.available())
	{
		String msg = readStringLn(&dualStream);
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
			dualStream.println("TestBefehle");
			dualStream.println("");
			dualStream.println("Befehle müssen /cmd [true|false|integer] geschrieben werden");
			dualStream.println("  deepSleepFlag : DeepSleep, Wake unbekannt");
			dualStream.println("  sleeptimer    : Schläft für x|20 sekunden");
			dualStream.println("  sleepkey      : Bei Tastendruck");
			dualStream.println("  gpio25        : Schaltet led");
			dualStream.println("  help          : Diese hilfe");
			return;
		}
		else if (msg.startsWith("sleeptimer")) {
			char wakeup_time_sec_txt[8];
			msg.substring(10).toCharArray(wakeup_time_sec_txt, sizeof(wakeup_time_sec_txt));
			int wakeup_time_sec = atoi(wakeup_time_sec_txt);
			if (wakeup_time_sec == 0)
				wakeup_time_sec = 20;
			dualStream.printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
			delay(15);
			esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
			esp_deep_sleep_start();
			return;
		}
		else if (msg == "sleepkey") {
			dualStream.println("Enabling ext wakeup");
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
		dualStream.print("interrupt_GPIO0->digitalRead(KEY_BUILTIN)==LOW");
		if(digitalRead(LED)==HIGH) {
			dualStream.print("->digitalRead(LED)==HIGH");
			deepsleepflag=true;
		}
		dualStream.println();
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

	char charBuf[7] = ""; //-23.56\0
	chat.topInfo = "RSSI " + String(LoRa.packetRssi(), DEC) + "  SNR " + dtostrf(LoRa.packetSnr(), sizeof(charBuf)-1, 2, charBuf);
	chat.received(rcvdMsg += "\0");
}

void send(String msg)
{
	LoRa.beginPacket();
	LoRa.print(msg);
	LoRa.endPacket();
	LoRa.receive();
	chat.sent(msg);
}
