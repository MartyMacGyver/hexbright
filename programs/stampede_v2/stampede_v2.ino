//#include <print_power.h>

// These next two lines must come after all other library #includes
#define BUILD_HACK
#include <hexbright.h>

//#include <Wire.h>

hexbright hb;

// Our flashlight mode definitions
#define OFF_MODE 0
#define ON_MODE 1
#define AIRPLANE_MODE 2
#define LOW_POWER 3

// Current flashlight state
//int current_brightness = MAX_LEVEL;
// set current brightness to half brightness for now to make it so not rediculously bright when turning on
int current_brightness = 500;
int onTime = 0;
int lowPowerTime = 0;

// State Flags
bool scaleBackwards = true;        // Do we need to scale brightness in
                                    // the opposite direction?

void setup() {
  hb.init_hardware();
}

void loop() {
  hb.update();
  
  // State variables for loop
  static int mode = OFF_MODE;      // Keep track of our current mode
  static bool changing_brightness = false;    // Are we updating the brightness
  static bool strobing = false;    // Are we strobing?
  
  // Enable seizure mode
  if (hb.button_released_time() > 50 && hb.button_released_time() < 250) {
    if (hb.button_just_pressed()) {
      if (!strobing)
        strobing = true;
      else {
        strobing = false;
        // Force it to go back to on mode
        mode = OFF_MODE;
      }
    }
  }
  
  // Brightness scale change
  if (hb.button_pressed() && hb.button_pressed_time() > 500) {
    if (mode == ON_MODE && hb.button_pressed()) {
      scale_brightness();
      changing_brightness = true;
    }
  }
  
  // Only switch state if button has just been relased
  // and we aren't currently changing brightness
  else if (hb.button_just_released() && !changing_brightness) {
    switch (mode) {
      case OFF_MODE:
        mode = change_mode(ON_MODE);
        break;
      case ON_MODE:
        mode = change_mode(OFF_MODE);
        break;
      case LOW_POWER:
        lowPowerTime = 0;
        onTime = millis();
        mode = change_mode(ON_MODE);
        break;
    }
  }
  
  // We were changing brightness, we just finished so reset the flag
  else
    changing_brightness = false;
  
  // Are we in seizure mode, if so strobe the light
  if (strobing) {
    do_strobe();
  }
  
  // If the light is on, do checks to see if we are in a low power state
  // and if we are give a low power warning
  if (mode == ON_MODE)
    display_battery_low();
  
  display_charge();
  
  // Check to see if we are in low power mode, and if we are
  // change it to off after being in low power for 2 minutes
  if (mode == LOW_POWER && millis() >= lowPowerTime + 120000) {
    mode = change_mode(OFF_MODE);
  }
  
  // check to see if light has been on for 5 mins
  // if it has, put it into low power mode
  if (millis() >= onTime + 300000 && mode == ON_MODE) {
    mode = change_mode(LOW_POWER);
  }
}

// Function to strobe the light
void do_strobe() {
  static unsigned long flash_time = millis();
  if(flash_time+70<millis()) { // flash every 70 milliseconds
    flash_time = millis(); // reset flash_time
    if (!hb.low_voltage_state())
      hb.set_light(MAX_LEVEL, 0, 10);
    else
      hb.set_light(MAX_LOW_LEVEL, 0, 10);
  }
}

// Function to scale the light brightness
//void scale_brightness() {
//  if (current_brightness >= 50 && current_brightness <= MAX_LEVEL) {
//    hb.set_light(CURRENT_LEVEL, current_brightness, 1);
//    if (!scaleBackwards)
//      current_brightness -= 4;
//    else
//      current_brightness += 4;
//  } else {
//    if (scaleBackwards) {
//      scaleBackwards = false;
//      if (!hb.low_voltage_state())
//        current_brightness = MAX_LEVEL;
//      else
//        current_brightness = MAX_LOW_LEVEL;
//    } else {
//      scaleBackwards = true;
//      current_brightness = 50;
//    }
//  }
//}

void scale_brightness() {
  if (hb.low_voltage_state()) {
    if (current_brightness >= 50 && current_brightness <= MAX_LOW_LEVEL) {
      hb.set_light(CURRENT_LEVEL, current_brightness, 1);
      if (!scaleBackwards)
        current_brightness -= 4;
      else
        current_brightness += 4;
    } else {
      if (scaleBackwards) {
        scaleBackwards = false;
        current_brightness = MAX_LOW_LEVEL;
      } else {
        scaleBackwards = true;
        current_brightness = 50;
      }
    }
  } else {
    if (current_brightness >= 50 && current_brightness <= MAX_LEVEL) {
      hb.set_light(CURRENT_LEVEL, current_brightness, 1);
      if (!scaleBackwards)
        current_brightness -= 4;
      else
        current_brightness += 4;
    } else {
      if (scaleBackwards) {
        scaleBackwards = false;
        current_brightness = MAX_LEVEL;
      } else {
        scaleBackwards = true;
        current_brightness = 50;
      }
    }
  }
}

// Function to change flashlight mode
int change_mode(const int newMode) {
  switch (newMode) {
    case ON_MODE:
      if (hb.low_voltage_state() && current_brightness > MAX_LOW_LEVEL)
        hb.set_light(CURRENT_LEVEL, MAX_LOW_LEVEL, 250);
      else
        hb.set_light(CURRENT_LEVEL, current_brightness, 250);
      onTime = millis();
      lowPowerTime = 0;
      break;
    case OFF_MODE:
      hb.set_light(CURRENT_LEVEL, OFF_LEVEL, 250);
      if (!hb.low_voltage_state())
        current_brightness = 500;
      else
        current_brightness = MAX_LOW_LEVEL;
      break;
    case LOW_POWER:
      onTime = 0;
      lowPowerTime = millis();
      hb.set_light(CURRENT_LEVEL, 0, 1);
      break;
  }
  return newMode;
}

// Function to give low battery warning
void display_battery_low() {
  // If in low voltage state, blink red LED to signal low battery
  if (hb.low_voltage_state() && hb.get_led_state(RLED) == LED_OFF) {
    hb.set_led(RLED, 50, 5000);    // Blink for 50ms every 5000ms
  }
}

// Function to display charging state.
void display_charge() {
  // Flash red when recharging and go solid green when full
  if (hb.get_charge_state() == CHARGING)
  //if (hb.get_charge_state() == CHARGING && hb.get_led_state(RLED) == LED_OFF)
    //hb.set_led(RLED, 500, 750);
    hb.set_led(RLED, 50);
  else if (hb.get_charge_state() == CHARGED)
    hb.set_led(GLED, 50);
}
