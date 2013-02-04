#include <hexbright.h>
#include <Wire.h>

#define OFF_MODE 0
#define ON_MODE 1

int mode = OFF_MODE;

hexbright hb;

void setup() {
  hb.init_hardware();
}

void loop() {
  hb.update();
  
  if (hb.button_just_pressed()) {
    switch (mode) {
      case OFF_MODE:
        if (!hb.low_voltage_state())
          hb.set_light(CURRENT_LEVEL, MAX_LEVEL, 5);
        else
          hb.set_light(CURRENT_LEVEL, MAX_LOW_LEVEL, 5);
        mode = ON_MODE;
        break;
      case ON_MODE:
        hb.set_light(CURRENT_LEVEL, OFF_LEVEL, 5);
        mode = OFF_MODE;
        break;
    }
  }
  
  if (hb.low_voltage_state())
    hb.print_charge(RLED);
  else
    hb.print_charge(GLED);
}

