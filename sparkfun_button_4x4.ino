#include "Tlc5940.h"

struct Color {
  byte r, g, b;
  Color(): r(0), g(0), b(0) {}
  Color(byte _r, byte _g, byte _b): r(_r), g(_g), b(_b) {}
};

struct LedState {
  Color start_color;
  Color target_color;
  Color current;
  unsigned long fade_time_start;
  float fade_time_length;
  bool fade_active;
  void FadeTo(Color c, float time) {
    start_color = current;
    target_color = c;
    fade_time_length = time;
    fade_time_start = millis();
    fade_active = true;
  }
  LedState(): fade_active(false), fade_time_length(0), fade_time_start(0) {}
};

volatile LedState leds[4][4];

void update_led(volatile void* led_p, unsigned long time) {
  // Hack!
  // For some reason Arduino does not like passing in structs as parameters?
  volatile LedState* led = (LedState*)led_p;
  float percent = (float)(time - led->fade_time_start) / led->fade_time_length;
  if (percent > 1) {
    // End
    led->current.r = led->target_color.r;
    led->current.g = led->target_color.g;
    led->current.b = led->target_color.b;
    led->fade_active = false;
  } else {
    led->current.r = led->start_color.r + ( ((int)(led->target_color.r) - (int)(led->start_color.r)) * percent);
    led->current.g = led->start_color.g + ( ((int)(led->target_color.g) - (int)(led->start_color.g)) * percent);
    led->current.b = led->start_color.b + ( ((int)(led->target_color.b) - (int)(led->start_color.b)) * percent);
  }
}

void setup()
{
  Tlc.init();
  set_XLAT_interrupt();
  tlc_onUpdateFinished = next_row;
  
  leds[0][0].FadeTo(Color(255, 0, 0), 3000);
}

void loop()
{
  // Update fading
  unsigned long time = millis();
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      if(leds[i][j].fade_active)
        update_led(&(leds[i][j]), time);
}

// Interrupt to shift out next row.
volatile uint8_t disp_row = 0;
volatile void next_row() {
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

