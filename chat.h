#ifndef __CHAT_HPP__
#define __CHAT_HPP__

#include "Arduino.h"
#include "heltec.h"

class Chat
{
	#define ADVANCED_CHATSTYLE	// Zeigt Gesendet links, empfange rechts. Andernfalls alles Links mit Symbolen
	#define CHAT_LINES	5
	#define CHAT_Y_START 12

	#define SENT_TXT "<|>"
	#define RCVD_TXT ">|<"

	enum ALIGNMENT {
		ALIGNMENT_NONE,
		ALIGNMENT_SENT,
		ALIGNMENT_RCVD
	};
	enum ALIGNMENT_DISP {
		ALIGNMENT_DISP_NONE = OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER,
		ALIGNMENT_DISP_SENT = OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_LEFT,
		ALIGNMENT_DISP_RCVD = OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT
	};
	enum ALIGNMENT_X {
		ALIGNMENT_X_NONE = 64,
		ALIGNMENT_X_SENT = 0,
		ALIGNMENT_X_RCVD = 128
	};

	public:
		void init() {
			Heltec.display->clear();
			Heltec.display->setFont(ArialMT_Plain_10);
			#ifdef ADVANCED_CHATSTYLE
			topInfo = "Sent               Received" ;
			#else
			Heltec.display->setLogBuffer(CHAT_LINES, 50);
			topInfo = "Sent: " SENT_TXT "   Received: " RCVD_TXT;
			#endif
		}
		
		String topInfo;

		void sent(String msg) { addConversation(msg, ALIGNMENT_SENT); }
		void received(String msg) { addConversation(msg, ALIGNMENT_RCVD); }
		void note(String note) { addConversation(note, ALIGNMENT_NONE); }

		void process()
		{
			if (!newMsgs) return;
			newMsgs = false;

			if (topInfo) {
				Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
				Heltec.display->drawString(64, 0, topInfo);
			}
			#ifdef ADVANCED_CHATSTYLE
				for (int i = 0; i < CHAT_LINES; i++) {
					ALIGNMENT alignment = conversation[i].alignment;
					Heltec.display->setTextAlignment(getAlignmentDisp(alignment));
					Heltec.display->drawString(getAlignmentX(alignment), CHAT_Y_START + i*10, conversation[i].txt);
				}
			#else
				for (int i = 0; i < CHAT_LINES; i++) {
					String txt = getAlignmentText(conversation[i].alignment) + conversation[i].txt;
					Heltec.display->println(txt);
				}
				Heltec.display->drawLogBuffer(0, CHAT_Y_START);
			#endif
			Heltec.display -> display();
			Heltec.display -> clear();

			if (conversation[CHAT_LINES -1].alignment == ALIGNMENT_RCVD) {
				int state = digitalRead(LED);
				digitalWrite(LED, !state);
				delay(100);
				digitalWrite(LED, state);
			}
		}

	private:
		
		struct Conversation {
			String txt;
			ALIGNMENT alignment;
		};

		Conversation conversation[CHAT_LINES];
		bool newMsgs = false;
		void addConversation(String msg, ALIGNMENT alignment)
		{
			for (int i = 1; i < CHAT_LINES; i++)
				conversation[i -1] = conversation[i];
			conversation[CHAT_LINES -1].txt = msg;
			conversation[CHAT_LINES -1].alignment = alignment;
			newMsgs = true;
			Serial.println(getAlignmentText(alignment) + " " + msg);
		}

		String getAlignmentText(ALIGNMENT alignment)
		{
			switch(alignment)
			{
				case ALIGNMENT_RCVD: return RCVD_TXT;
				case ALIGNMENT_SENT: return SENT_TXT;
				case ALIGNMENT_NONE:
				default:
					return "";
			}
		}

		OLEDDISPLAY_TEXT_ALIGNMENT getAlignmentDisp(ALIGNMENT alignment)
		{
			switch(alignment)
			{
				case ALIGNMENT_RCVD: return (OLEDDISPLAY_TEXT_ALIGNMENT)ALIGNMENT_DISP_RCVD;
				case ALIGNMENT_SENT: return (OLEDDISPLAY_TEXT_ALIGNMENT)ALIGNMENT_DISP_SENT;
				case ALIGNMENT_NONE:
				default:
					return (OLEDDISPLAY_TEXT_ALIGNMENT)ALIGNMENT_DISP_NONE;
			}
		}

		uint16_t getAlignmentX(ALIGNMENT alignment)
		{
			switch(alignment)
			{
				case ALIGNMENT_RCVD: return ALIGNMENT_X_RCVD;
				case ALIGNMENT_SENT: return ALIGNMENT_X_SENT;
				case ALIGNMENT_NONE:
				default:
					return ALIGNMENT_X_NONE;
			}
		}
};

#endif