#include <hexbright.h>
#include <Wire.h>

hexbright hb;

// Our flashlight mode definitions
#define OFF_MODE 0
#define ON_MODE 1
#define SEIZURE_MODE 2

// Current flashlight state
int mode = OFF_MODE;
int current_brightness = MAX_LEVEL;

// Debugging Mode
bool debug = false;

// Do we need to switch modes?
bool switchMode = false;
bool brightnessChanged = false;

void setup() {
  hb.init_hardware();
}

void loop() {
  hb.update();
  
  // Brightness scale change
  if (hb.button_pressed() && hb.button_pressed_time() > 500) {
    if (mode == ON_MODE && hb.button_pressed()) {
      if (current_brightness >= 50) {
        hb.set_light(CURRENT_LEVEL, current_brightness, 1);
        current_brightness -= 5;
      }
      brightnessChanged = true;
    }
  }
  
  if (hb.button_just_released() && hb.button_pressed_time() > 115) {
    switch (mode) {
      case OFF_MODE:
        mode = ON_MODE;
        switchMode = true;
        break;
      case ON_MODE:
        if (!brightnessChanged) {
          mode = OFF_MODE;
          switchMode = true;
        } else {
          mode = ON_MODE;
          brightnessChanged = false;
        }
        break;
    }
  }
  
  if (switchMode) {
    switch (mode) {
      case ON_MODE:
        if (hb.low_voltage_state() && hb.get_max_light_level() > MAX_LOW_LEVEL)
          hb.set_light(CURRENT_LEVEL, MAX_LOW_LEVEL, 5);
        else
          hb.set_light(CURRENT_LEVEL, current_brightness, 5);
        break;
      case OFF_MODE:
        hb.set_light(CURRENT_LEVEL, OFF_LEVEL, 5);
        current_brightness = MAX_LEVEL;
        break;
    }
    switchMode = false;
  }
  
  if (debug)
    hb.print_charge(RLED);
  else
    hb.print_charge(GLED);
}

