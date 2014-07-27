#include <Arduino.h>
#include <Wire.h>
#include "ButtonPad.h"

const int WIRE_ADDRESS = 157;

void write_long(uint32_t time) {
	Wire.write((uint8_t*)&time, sizeof(unsigned long));
}

ButtonPad::ButtonPad(int pin)
: interrupt_pin(pin)
{}

void ButtonPad::begin() {
	pinMode(INPUT, interrupt_pin);
	Wire.begin();
}

void ButtonPad::clearAll() const {
	Wire.beginTransmission(WIRE_ADDRESS);
	Wire.write(Command_ClearAll);
	Wire.endTransmission();
}

void ButtonPad::clearRect(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom) const {
	Wire.beginTransmission(WIRE_ADDRESS);
	Wire.write(Command_ClearRectangle);
	Wire.write(top);
	Wire.write(bottom);
	Wire.write(left);
	Wire.write(right);
	Wire.endTransmission();
}

void ButtonPad::ledSolid(const Point& p, const Color& c) const {
	Wire.beginTransmission(WIRE_ADDRESS);
	Wire.write(Command_SetLed);
	Wire.write(1);
	Wire.write(p.y);
	Wire.write(p.x);
	Wire.write(Fade_Solid);
	Wire.write(c.r);
	Wire.write(c.g);
	Wire.write(c.b);
	Wire.endTransmission();
}

void ButtonPad::ledFade(const Point& p, const Color& c, uint32_t time) const {
	Wire.beginTransmission(WIRE_ADDRESS);
	Wire.write(Command_SetLed);
	Wire.write(1);
	Wire.write(p.y);
	Wire.write(p.x);
	Wire.write(Fade_Once);
	Wire.write(c.r);
	Wire.write(c.g);
	Wire.write(c.b);
	write_long(time);
	Wire.endTransmission();
}

void ButtonPad::ledBounce(const Point& p, const Color& from, const Color& to, uint32_t time) const {
	Wire.beginTransmission(WIRE_ADDRESS);
	Wire.write(Command_SetLed);
	Wire.write(1);
	Wire.write(p.y);
	Wire.write(p.x);
	Wire.write(Fade_Bounce);
	Wire.write(from.r);
	Wire.write(from.g);
	Wire.write(from.b);
	Wire.write(to.r);
	Wire.write(to.g);
	Wire.write(to.b);
	write_long(time);
	Wire.endTransmission();
}

bool ButtonPad::update() {
	if (digitalRead(interrupt_pin)) {
		Wire.requestFrom(WIRE_ADDRESS, sizeof(key_state));
		Wire.readBytes(static_cast<char*>(static_cast<void*>(key_state)), sizeof(key_state));
		return true;
	}
	return false;
}

bool ButtonPad::keyPressed() const {
	return key_state[0].kstate == ButtonPad::Key::PRESSED && key_state[0].stateChanged;
}

Point ButtonPad::getPressed() const {
	int num = key_state[0].kchar;
	return Point((num - 1) / 4, (num - 1) % 4);
}