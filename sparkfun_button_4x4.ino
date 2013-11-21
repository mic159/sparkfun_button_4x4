#include "Tlc5940.h"
#include <Keypad.h>
#include <Wire.h>

#define SWAP(type, x, y) {type tmp = x; x = y; y = tmp;}

const int INT_PIN = 4;
const int WIRE_ADDRESS = 157;
// Wire has a max of 32 bytes, a Key is 4 bytes each
// so we cannot fit the full 10 in a single transmit.
// Compromise at 5
const int KEYS_TX_SIZE = 5;

struct Color {
  byte r, g, b;
  // Constructors
  Color(): r(0), g(0), b(0) {}
  Color(byte _r, byte _g, byte _b): r(_r), g(_g), b(_b) {}
  Color(const Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  // volatile versions
  Color(volatile Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  Color& operator=(volatile const Color& rhs) volatile {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
  }
};

enum FadeMode {
  Fade_Solid,
  Fade_Once,
  Fade_Bounce,

  Fade_Max,
};

struct LedState {
  Color start_color;
  Color target_color;
  Color current;
  FadeMode fade_mode;
  unsigned long fade_time_start;
  unsigned long fade_time_length;

  void FadeTo(const Color& c, unsigned long time) volatile {
    start_color = current;
    target_color = c;
    fade_time_length = time;
    fade_time_start = millis();
    fade_mode = Fade_Once;
  }

  void FadeBounce(const Color& start, const Color& end, unsigned long time) volatile {
    start_color = start;
    target_color = end;
    fade_time_length = time;
    fade_time_start = millis();
    fade_mode = Fade_Bounce;
  }

  void SetColor(const Color& c) volatile {
    current = c;
    fade_mode = Fade_Solid;
  }

  void Update(unsigned long time) volatile {
    if (fade_mode == Fade_Solid || fade_mode > Fade_Max) return;
    float percent = (float)(time - fade_time_start) / fade_time_length;
    if (percent > 1) {
      // End
      current = target_color;
      if (fade_mode == Fade_Bounce) {
        SWAP(Color, start_color, target_color);
        fade_time_start = time;
      } else {
        fade_mode = Fade_Solid;
      }
    } else {
      current.r = start_color.r + ( ((int)(target_color.r) - (int)(start_color.r)) * percent);
      current.g = start_color.g + ( ((int)(target_color.g) - (int)(start_color.g)) * percent);
      current.b = start_color.b + ( ((int)(target_color.b) - (int)(start_color.b)) * percent);
    }
  }
  LedState(): fade_mode(Fade_Solid), fade_time_length(0), fade_time_start(0) {}
};

const uint8_t KEYPAD_ROWS=4;
const uint8_t KEYPAD_COLS=4;
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
byte rowPins[KEYPAD_ROWS] = {5, 6, 7, 8};
byte colPins[KEYPAD_COLS] = {A0, A1, A2, A3};
Keypad buttons = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
volatile LedState leds[KEYPAD_ROWS][KEYPAD_COLS];

void setup()
{
  pinMode(INT_PIN, OUTPUT);
  Wire.begin(WIRE_ADDRESS);
  Wire.onReceive(handle_action);
  Wire.onRequest(send_buttons);
  Tlc.init();
  set_XLAT_interrupt();
  tlc_onUpdateFinished = next_row;
}

void send_buttons() {
  Wire.write((byte*)buttons.key, sizeof(Key) * KEYS_TX_SIZE);
  digitalWrite(INT_PIN, LOW);
}

// Helper function to get a long from the Wire library
unsigned long read_long() {
  char buff[sizeof(unsigned long)];
  Wire.readBytes(buff, sizeof(unsigned long));
  return *((unsigned long*)buff);
}

void update_led(volatile LedState& led, byte action) {}

void handle_action(int bytes) {
  // Unwrap command
  byte command = Wire.read(); --bytes;
  if (command == 0) {
    for (int y = 0; y < KEYPAD_ROWS; ++y)
      for (int x = 0; x < KEYPAD_COLS; ++x)
        leds[y][x].SetColor(Color());
    return;
  }
  if (bytes > 3) {
    byte action = Wire.read();
    byte row = Wire.read();
    byte col = Wire.read();
    // Make future checks easier
    bytes -= 3;

    volatile LedState& led = leds[row][col];
    switch (action) {
      case Fade_Solid:
        if (bytes < sizeof(Color)) break;
        led.SetColor(
            Color(Wire.read(), Wire.read(), Wire.read())
          );
        break;
      case Fade_Once:
        if (bytes < sizeof(Color) + sizeof(unsigned long)) break;
        led.FadeTo(
            Color(Wire.read(), Wire.read(), Wire.read())
          , read_long()
          );
        break;
      case Fade_Bounce:
        if (bytes < sizeof(Color) * 2 + sizeof(unsigned long)) break;
        led.FadeBounce(
            Color(Wire.read(), Wire.read(), Wire.read())
          , Color(Wire.read(), Wire.read(), Wire.read())
          , read_long()
          );
        break;
    }
  }

  // Consume anything still in the buffer...
  while (Wire.available()) {
    byte _ = Wire.read();
  }
}

void loop()
{
  // Update fading
  unsigned long time = millis();
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      leds[i][j].Update(time);

  // Read in keypad to internal buffer.
  bool changed = buttons.getKeys();
  if (changed) {
    digitalWrite(INT_PIN, HIGH);
  }

  delay(1);
}

// Interrupt to shift out next row.
volatile uint8_t disp_row = 0;
volatile uint8_t disp_frame = 0;

volatile void next_row() {
  // 5 frames per row
  disp_frame = (disp_frame + 1) % 2;
  if (disp_frame != 0) {
    Tlc.update();
    return;
  }

  Tlc.clear();
  Tlc.set(12+disp_row, 4095);
  for (int led = 0; led < 4; led++) {
    // The value needs to be 0-4095, so multiply by 16.
    Tlc.set((0 * 4) + led, leds[disp_row][led].current.b * 16);
    Tlc.set((1 * 4) + led, leds[disp_row][led].current.g * 16);
    Tlc.set((2 * 4) + led, leds[disp_row][led].current.r * 16);
  }
  disp_row = (disp_row + 1) % 4;
  Tlc.update();
}

