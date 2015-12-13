#include "Stampede.h"

#define BUILD_HACK
#include <hexbright.h>

Stampede::Stampede(hexbright *hb) {
  currentBrightness = 500;
  onTime = 0;
  lowPowerTime = 0;
  scaleBackwards = true;
  mode = OFF_MODE;
  changingBrightness = false;
  strobing = false;
  hbLib = hb;
  tapCount = 0;
  lastTap = 0;

  if (hbLib->get_charge_state() == BATTERY)
    powerSaveEnabled = true;
  else
    powerSaveEnabled = false;

}

int Stampede::pollStates() {
  //Serial.print("Polling States\n");
  int newMode = mode;

  // Check for taps
  if (hbLib->tapped()) {
    //Serial.print("Tap detected\n");
    if (millis() >= lastTap + 100) {
      lastTap = millis();
      tapCount = tapCount + 1;
      //Serial.print("tapCount is: ");
      //Serial.print(tapCount);
      //Serial.print("\n");
    }
  }

  if (millis() > lastTap + 1000) {
    if (tapCount == 3 && hbLib->get_charge_state() == BATTERY) {
      if (powerSaveEnabled) {
        powerSaveEnabled = false;
      }
      else {
        powerSaveEnabled = true;
      }
    }

    if (tapCount != 0) {
      //Serial.print("tapCount reset to 0\n");
      tapCount = 0;
    }
  }

  // Check if seizure mode is being asked for
  if (hbLib->button_released_time() > 50 && hbLib->button_released_time() < 250) {
    if (hbLib->button_just_pressed()) {
      if (!strobing) {
        //Serial.print("Strobe On\n");
        strobing = true;
      }
      else {
        //Serial.print("Strobe Off\n");
        strobing = false;
        // Force it to go back to on mode
        // Setting newMode to OFF_MODE because button_just_released() will
        // return true and execute below. This will cause it to just flip back
        // to ON_MODE once the code finishes executing.
        newMode = OFF_MODE;
      }
    }
  }

  // Brightness scale change
  if (hbLib->button_pressed() && hbLib->button_pressed_time() > 500) {
    //Serial.print("Changing brightness\n");
    if (mode == ON_MODE && hbLib->button_pressed()) {
      changingBrightness = true;
    }
  }

  if (hbLib->button_just_released()) {
    if (!changingBrightness) {
      switch (mode) {
        case OFF_MODE:
          //Serial.print("Switching from OFF_MODE to ON_MODE\n");
          newMode = ON_MODE;
          break;
        case ON_MODE:
          //Serial.print("Switching from ON_MODE to OFF_MODE\n");
          newMode = OFF_MODE;
          break;
        case LOW_POWER:
          //Serial.print("Switching from LOW_POWER to ON_MODE\n");
          newMode = ON_MODE;
          break;
      }
    }
    else
      changingBrightness = false;
  }

  // Check to see if we are in low power mode, and if we are
  // change it to off after being in low power for 2 minutes
  if (mode == LOW_POWER && millis() >= lowPowerTime + 120000) {
    //Serial.print("Switching from LOW_POWER to OFF_MODE\n");
    newMode = OFF_MODE;
  }

  // check to see if light has been on for 5 mins
  // if it has, put it into low power mode
  if (millis() >= onTime + 300000 && mode == ON_MODE) {
    if (powerSaveEnabled) {
      //Serial.print("Switching from ON_MODE to LOW_POWER\n");
      newMode = LOW_POWER;
    }
  }

  return newMode;
}

void Stampede::doTick() {
  //Serial.print("Start clock tick\n");
  int newMode;

  // Poll states of buttons to setup for proper state
  newMode = pollStates();

  if (changingBrightness)
    scaleBrightness();

  if (strobing)
    doStrobe();

  // Update flashlight mode if mode has changed this clock tick
  if (newMode != mode)
    changeMode(newMode);

  // Check for battery low or charging states
  displayBatteryLow();
  displayCharge();
  displayPowerSave();

  //Serial.print("End clock tick\n");
}

void Stampede::doStrobe() {
  //Serial.print("Doing strobe\n");
  static unsigned long flash_time = millis();
  if (flash_time + 70 < millis()) { // flash every 70 milliseconds
    flash_time = millis(); // reset flash_time
    if (!hbLib->low_voltage_state())
      hbLib->set_light(MAX_LEVEL, 0, 10);
    else
      hbLib->set_light(MAX_LOW_LEVEL, 0, 10);
  }
}

void Stampede::scaleBrightness() {
  //Serial.print("Scaling brightness\n");
  if (hbLib->low_voltage_state()) {
    if (currentBrightness >= 50 && currentBrightness <= MAX_LOW_LEVEL) {
      hbLib->set_light(CURRENT_LEVEL, currentBrightness, 1);
      if (!scaleBackwards)
        currentBrightness -= 4;
      else
        currentBrightness += 4;
    } else {
      if (scaleBackwards) {
        scaleBackwards = false;
        currentBrightness = MAX_LOW_LEVEL;
      } else {
        scaleBackwards = true;
        currentBrightness = 50;
      }
    }
  } else {
    if (currentBrightness >= 50 && currentBrightness <= MAX_LEVEL) {
      hbLib->set_light(CURRENT_LEVEL, currentBrightness, 1);
      if (!scaleBackwards)
        currentBrightness -= 4;
      else
        currentBrightness += 4;
    } else {
      if (scaleBackwards) {
        scaleBackwards = false;
        currentBrightness = MAX_LEVEL;
      } else {
        scaleBackwards = true;
        currentBrightness = 50;
      }
    }
  }
}

void Stampede::changeMode(const int newMode) {
  //Serial.print("Switching flashlight mode\n");
  switch (newMode) {
    case ON_MODE:
      if (hbLib->low_voltage_state() && currentBrightness > MAX_LOW_LEVEL)
        hbLib->set_light(CURRENT_LEVEL, MAX_LOW_LEVEL, 250);
      else
        hbLib->set_light(CURRENT_LEVEL, currentBrightness, 250);
      onTime = millis();
      lowPowerTime = 0;
      break;
    case OFF_MODE:
      hbLib->set_light(CURRENT_LEVEL, OFF_LEVEL, 250);
      if (!hbLib->low_voltage_state())
        currentBrightness = 500;
      else
        currentBrightness = MAX_LOW_LEVEL;
      break;
    case LOW_POWER:
      onTime = millis();
      lowPowerTime = millis();
      hbLib->set_light(CURRENT_LEVEL, 0, 1);
      break;
  }
  mode = newMode;
}

void Stampede::displayBatteryLow() {
  //Serial.print("Displaying battery low\n");
  // If in low voltage state, blink red LED to signal low battery
  if (hbLib->low_voltage_state() && hbLib->get_led_state(RLED) == LED_OFF) {
    hbLib->set_led(RLED, 50, 5000);    // Blink for 50ms every 5000ms
  }
}

void Stampede::displayCharge() {
  //Serial.print("Displaying charge\n");
  // Flash red when recharging and go solid green when full
  if (hbLib->get_charge_state() == CHARGING)
    //if (hb.get_charge_state() == CHARGING && hb.get_led_state(RLED) == LED_OFF)
    //hb.set_led(RLED, 500, 750);
    hbLib->set_led(RLED, 50);
  else if (hbLib->get_charge_state() == CHARGED)
    hbLib->set_led(GLED, 50);
}

void Stampede::displayPowerSave() {
  if (powerSaveEnabled)
    hbLib->set_led(GLED, 10, 0);
}

void Stampede::doUpdate() {
  hbLib->update();
}

Stampede *stampede;

void setup() {
  //Serial.begin(19200);
  //Serial.print("Initializing\n");
  // put your setup code here, to run once:
  hexbright hb;
  hb.init_hardware();
  stampede = new Stampede(&hb);
}

void loop() {
  //Serial.print("Loop Begin\n");
  // put your main code here, to run repeatedly:
  stampede->doUpdate();
  stampede->doTick();
}

