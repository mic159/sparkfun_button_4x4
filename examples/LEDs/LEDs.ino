#include <Wire.h>
#include <ButtonPad.h>

ButtonPad pad(8);

void setup() {
	pad.begin();
	pad.clearAll();
};

void loop() {
	pad.clearAll();
	delay(100);
	pad.ledSolid(Point(0, 0), Color(255,0,0));
	pad.ledSolid(Point(1, 0), Color(0,255,0));
	pad.ledSolid(Point(2, 0), Color(0,0,255));
	pad.ledFade(Point(0, 1), Color(255,0,0), 1000);
	pad.ledFade(Point(1, 1), Color(0,255,0), 1000);
	pad.ledFade(Point(2, 1), Color(0,0,255), 1000);
	pad.ledBounce(Point(0, 2), Color(0,0,0), Color(255,0,0), 1000);
	pad.ledBounce(Point(1, 2), Color(0,0,0), Color(0,255,0), 1000);
	pad.ledBounce(Point(2, 2), Color(0,0,0), Color(0,0,255), 1000);
	delay(5000);
};