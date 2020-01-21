#ifndef __UTILS_H__
#define __UTILS_H__

#include "Arduino.h"
#include "heltec.h"


String readStringLn() {	return Serial.readStringUntil('\n'); }

void writeString(uint16_t x, uint16_t y, int linelen, String txt) {
	String _txt = txt;
	_txt.remove(linelen);
	Heltec.display->drawString(x, y, _txt);
	int i = 1;
	while (txt.length() > linelen) {
		_txt = txt = txt.substring(linelen);
		_txt.remove(linelen);
		Heltec.display->drawString(x, y + 10*i++, _txt);
	}
}

enum ENDS_WITH {
	ENDS_WITH_FALSE = 0,
	ENDS_WITH_TRUE = 1,
	ENDS_WITH_NONE = -1
};
int txtEndsWithBool(String txt, String cmd) {
	txt.toLowerCase();
	if (txt.startsWith(cmd))
	{
		if (txt.endsWith("true")) {
			Serial.println("# " + cmd + " = true");
			return ENDS_WITH_TRUE;
		} else if (txt.endsWith("false")) {
			Serial.println("# " + cmd + " = false");
			return ENDS_WITH_FALSE;
		}
	}
	return ENDS_WITH_NONE;
}

#endif
