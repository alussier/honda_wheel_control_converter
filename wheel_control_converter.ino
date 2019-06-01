/*
   lorem ipsum
*/

#include <IRremote.h>

#ifndef ARDUINO_AVR_PROMICRO
#error "Pro Micro not selected (Tools -> Board).  Check IRRemote library support before changing board"
#endif

#define SERIAL_CONSOLE         (1)
#define WHEEL_OHM_TOLERANCE    (100)
#define DEBOUNCE_DELAY         (50)

#define VCC                    (5.0)
#define R2                     (1000.0)
#define WHEEL_PIN              (A0)

enum button                   {NONE = 0, CHUP = 1, CHDOWN = 2, VOLUP = 3, VOLDOWN = 4, MODE = 5};
unsigned int wheel_ohm[6] =   {10366,    1740,     794,        367,       103,         3835    };
unsigned long jvc_button[6] = {0,        0xF149,   0xF1C9,     0xF121,    0xF1A1,      0xF111  };

enum button last_button;
unsigned int last_debounce;

IRsend irsend;

// the setup routine runs once when you press reset:
void setup() {
#ifdef SERIAL_CONSOLE
  Serial.begin(9600);
#endif
  last_button = NONE;
  last_debounce = millis();
}

// the loop routine runs over and over again forever:
void loop() {
  enum button input = get_button();

  if (input != last_button) {
    last_debounce = millis();
  }

  if ((millis() - last_debounce) > DEBOUNCE_DELAY) {
    xmit(input);
    delay(2 * DEBOUNCE_DELAY);
  }
}

void xmit(enum button button) {
  if (!button) {
    return;
  }
#ifdef SERIAL_CONSOLE
  Serial.println(button);
#endif
  // send two 16-bit packets, one with repeat flag
  irsend.sendJVC(jvc_button[button], 16, 0);
  delayMicroseconds(50);
  irsend.sendJVC(jvc_button[button], 16, 1);
  delayMicroseconds(50);
}

float get_voltage() {
  int v = analogRead(WHEEL_PIN);
  // scale 0-1023 input to VCC
  float voltage = v * (VCC / 1023.0);
  return voltage;
}

float get_resistance() {
  // voltage divider
  float r = (R2 * (VCC / get_voltage())) - R2;
  return r;
}

enum button get_button() {
  float r = get_resistance();
  for (int i = 0; i < sizeof(wheel_ohm) / sizeof(wheel_ohm[0]); i++) {
    if (abs(wheel_ohm[i] - r) < WHEEL_OHM_TOLERANCE) {
      return (button)i;
    }
  }
  return NONE;
}
