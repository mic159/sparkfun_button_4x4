#ifndef KEYPAD_4_H
#define KEYPAD_4_H


struct Color {
  uint8_t r, g, b;
  // Constructors
  Color(): r(0), g(0), b(0) {}
  Color(uint8_t _r, uint8_t _g, uint8_t _b): r(_r), g(_g), b(_b) {}
  Color(const Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  // volatile versions
  Color(volatile Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  Color& operator=(volatile const Color& rhs) volatile {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
  }
};

struct Point {
	uint8_t x, y;
	Point() : x(0), y(0) {}
	Point(uint8_t x, uint8_t y) : x(x), y(y) {}
	Point(const Point& o) : x(o.x), y(o.y) {}
};

class ButtonPad {
public:
	ButtonPad(int interrupt_pin);
	void begin();

	enum FadeMode {
		Fade_Solid,
		Fade_Once,
		Fade_Bounce,

		Fade_Max,
	};

	enum LedCommand {
		Command_ClearAll,
		Command_ClearRectangle,
		Command_SetLed,

		Command_Max,
	};

	struct Key {
		enum KeyState {
			IDLE,
			PRESSED,
			HOLD,
			RELEASED
		};
		char kchar;
	    KeyState kstate;
	    bool stateChanged;
	};

	void ledSolid(const Point& p, const Color& c) const;
	void ledFade(const Point& p, const Color& c, uint32_t millis) const;
	void ledBounce(const Point& p, const Color& from, const Color& to, uint32_t millis) const;

	void clearAll() const;
	void clearRect(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom) const;

	// Buttons
	bool update();
	bool keyPressed() const;
	Point getPressed() const;
private:
	int interrupt_pin;
	Key key_state[5];
};

#endif