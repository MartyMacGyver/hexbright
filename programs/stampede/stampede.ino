#include <hexbright.h>
#include <Wire.h>

hexbright hb;

// Our flashlight mode definitions
#define OFF_MODE 0
#define ON_MODE 1
#define SEIZURE_MODE 2
#define AIRPLANE_MODE 3

// Current flashlight state
int mode = OFF_MODE;
int current_brightness = MAX_LEVEL;

// State Flags
bool switchMode = false;            // Do we need to update our current mode?
bool brightnessChanged = false;     // Are we just changing brightness?
bool scaleBackwards = false;        // Do we need to scale brightness in
                                    // the opposite direction?
bool doStrobe = false;

void setup() {
  hb.init_hardware();
}

void loop() {
  hb.update();
  
  // Airplane Mode
  if (hb.button_pressed() && hb.button_pressed_time() > 1500) {
    if (mode == OFF_MODE) {
#if (DEBUG == DEBUG_PRINT)
      Serial.println("*** Switch Airplane Mode ***");
#endif
      mode = AIRPLANE_MODE;
    }
  }
  
  // Enable seizure mode
  if (hb.button_released_time() > 50 && hb.button_released_time() < 250) {
    if (hb.button_just_pressed()) {
      if (mode != SEIZURE_MODE)
        mode = SEIZURE_MODE;
      else
        mode = OFF_MODE;
        doStrobe = false;
    }
  }
  
  // Brightness scale change
  if (hb.button_pressed() && hb.button_pressed_time() > 500) {
    if (mode == ON_MODE && hb.button_pressed()) {
      if (current_brightness >= 50 && current_brightness <= MAX_LEVEL) {
        hb.set_light(CURRENT_LEVEL, current_brightness, 1);
        if (!scaleBackwards)
          current_brightness -= 4;
        else
          current_brightness += 4;
      } else {
        if (scaleBackwards) {
          scaleBackwards = false;
          if (!hb.low_voltage_state())
            current_brightness = MAX_LEVEL;
          else
            current_brightness = MAX_LOW_LEVEL;
        } else {
          scaleBackwards = true;
          current_brightness = 50;
        }
      }
      brightnessChanged = true;
    }
  }
  
  if (hb.button_just_released()) {
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
      case SEIZURE_MODE:
        if (!doStrobe)
          switchMode = true;
        break;
      case AIRPLANE_MODE:
        mode = OFF_MODE;
        break;
    }
  }
  
  if (switchMode) {
    switch (mode) {
      case ON_MODE:
        if (hb.low_voltage_state() && current_brightness > MAX_LOW_LEVEL)
          hb.set_light(CURRENT_LEVEL, MAX_LOW_LEVEL, 5);
        else
          hb.set_light(CURRENT_LEVEL, current_brightness, 5);
        break;
      case OFF_MODE:
        hb.set_light(CURRENT_LEVEL, OFF_LEVEL, 5);
        if (!hb.low_voltage_state())
          current_brightness = MAX_LEVEL;
        else
          current_brightness = MAX_LOW_LEVEL;
        break;
      case SEIZURE_MODE:
        doStrobe = true;
        break;
    }
    switchMode = false;
  }
  
  if (doStrobe) {
    static unsigned long flash_time = millis();
    if(flash_time+70<millis()) { // flash every 70 milliseconds
      flash_time = millis(); // reset flash_time
      if (!hb.low_voltage_state())
        hb.set_light(MAX_LEVEL, 0, 20);
      else
        hb.set_light(MAX_LOW_LEVEL, 0, 20);
    }
  }
  
  // If in low voltage state, blink red LED to signal low battery
  if (hb.low_voltage_state() && hb.get_led_state(RLED) == LED_OFF) {
    if (mode == ON_MODE)    // But only do this if the light is on
      hb.set_led(RLED, 50, 5000);    // Blink for 50ms every 5000ms
  }
  
  // Flash red when recharging and go solid green when full
  if (hb.get_charge_state() == CHARGING && hb.get_led_state(RLED) == LED_OFF)
    hb.set_led(RLED, 500, 750);
  else if (hb.get_charge_state() == CHARGED)
    hb.set_led(GLED, 50);
}

