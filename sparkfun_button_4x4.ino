#include "Tlc5940.h"
#include <Keypad.h>

#define SWAP(type, x, y) {type tmp = x; x = y; y = tmp;}

struct Color {
  byte r, g, b;
  // Constructors
  Color(): r(0), g(0), b(0) {}
  Color(byte _r, byte _g, byte _b): r(_r), g(_g), b(_b) {}
  Color(const Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  // volatile versions
  Color(volatile Color& rhs): r(rhs.r), g(rhs.g), b(rhs.b) {}
  Color& operator=(volatile Color& rhs) volatile {
    r = rhs.r;
    g = rhs.g;
    b = rhs.b;
  }
};

enum FadeMode {
  Fade_Off,
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
  float fade_time_length;

  void FadeTo(Color c, float time) volatile {
    start_color = current;
    target_color = c;
    fade_time_length = time;
    fade_time_start = millis();
    fade_mode = Fade_Once;
  }

  void FadeBounce(Color start, Color end, float time) volatile {
    start_color = start;
    target_color = end;
    fade_time_length = time;
    fade_time_start = millis();
    fade_mode = Fade_Bounce;
  }

  void Update(unsigned long time) volatile {
    if (fade_mode == Fade_Off || fade_mode > Fade_Max) return;
    float percent = (float)(time - fade_time_start) / fade_time_length;
    if (percent > 1) {
      // End
      current.r = target_color.r;
      current.g = target_color.g;
      current.b = target_color.b;
      if (fade_mode == Fade_Bounce) {
        SWAP(Color, start_color, target_color);
        fade_time_start = time;
      } else {
        fade_mode = Fade_Off;
      }
    } else {
      current.r = start_color.r + ( ((int)(target_color.r) - (int)(start_color.r)) * percent);
      current.g = start_color.g + ( ((int)(target_color.g) - (int)(start_color.g)) * percent);
      current.b = start_color.b + ( ((int)(target_color.b) - (int)(start_color.b)) * percent);
    }
  }
  LedState(): fade_mode(Fade_Off), fade_time_length(0), fade_time_start(0) {}
};

const uint8_t ROWS=4;
const uint8_t COLS=4;
char keys[ROWS][COLS] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {A0, A1, A2, A3};
Keypad buttons = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
volatile LedState leds[ROWS][COLS];

void setup()
{
  Tlc.init();
  set_XLAT_interrupt();
  tlc_onUpdateFinished = next_row;
}

void loop()
{
  // Update fading
  unsigned long time = millis();
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      leds[i][j].Update(time);

  char key = buttons.getKey();
  if (key && key <= 16) {
    key = key - 1;
    byte row = key % ROWS;
    byte col = key / COLS;
    volatile LedState& led = leds[row][col];
    if (led.fade_mode == Fade_Once) {
      led.FadeBounce(Color(0, 0, 0), Color(255, 0, 0), 1000);
    } else {
      if (led.current.g) {
        led.FadeTo(Color(0, 0, 0), 1000);
      } else {
        led.FadeTo(Color(0, 255, 0), 1000);
      }
    }
  }
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
    Tlc.set((0 * 4) + led, leds[disp_row][led].current.r * 16);
    Tlc.set((1 * 4) + led, leds[disp_row][led].current.g * 16);
    Tlc.set((2 * 4) + led, leds[disp_row][led].current.b * 16);
  }
  disp_row = (disp_row + 1) % 4;
  Tlc.update();
}

