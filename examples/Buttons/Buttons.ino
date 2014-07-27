#include <Wire.h>
#include <ButtonPad.h>

ButtonPad pad(8);

void setup() {
	Serial.begin(57600);
	pad.begin();
	pad.clearAll();
	pad.ledSolid(Point(0, 0), Color(0,255,0));
};

void loop() {
	if (pad.update() && pad.keyPressed()) {
		Point p = pad.getPressed();
		pad.ledSolid(p, Color(0, 0, 255));
	}
};