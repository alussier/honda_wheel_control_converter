/*
  ReadAnalogVoltage
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

// override library to modify output pin
#include "src/IRremote/IRremote.h"

#define WHEEL_PIN              A0
#define R2                     1000.0

#define NUM_BUTTONS            6
#define WHEEL_NONE             0
#define WHEEL_CHUP             1
#define WHEEL_CHDOWN           2
#define WHEEL_VOLUP            3
#define WHEEL_VOLDOWN          4
#define WHEEL_MODE             5

//                                       none   chup    chdown  volup   voldown  mode
unsigned int wheel_ohm[NUM_BUTTONS] =   {10366, 1740,   794,    367,    103,     3835};
unsigned int jvc_buttons[NUM_BUTTONS] = {0,     0xF149, 0xF1C9, 0xF121, 0xF1A1,  0xF111};

#define WHEEL_OHM_TOLERANCE    50
#define DEBOUNCE_DELAY         50

int last_button;
int last_debounce;

IRsend irsend;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  last_button = 0;
  last_debounce = millis();
}

// the loop routine runs over and over again forever:
void loop() {
  int input = get_button();

  if (input != last_button) {
    last_debounce = millis();
  }

  if ((millis() - last_debounce) > DEBOUNCE_DELAY) {
    xmit(input);
    delay(100);
  }
}

void xmit(int button) {
  if (button == WHEEL_NONE) {
    return;
  }
  Serial.println(button);
  // send two 16-bit packets, one with repeat flag
  irsend.sendJVC(jvc_buttons[button], 16, 0);
  delayMicroseconds(50);
  irsend.sendJVC(jvc_buttons[button], 16, 1);
  delayMicroseconds(50);
}

float get_voltage() {
  int x = analogRead(WHEEL_PIN);
  // scale 0-1023 input to 5V
  float voltage = x * (5.0 / 1023);
  return voltage;
}

float get_resistance(float v_out) {
  // voltage divider
  float r = (R2 * (5.0 / v_out)) - R2;
  return r;
}

int get_button() {
  int v = get_voltage();
  int r = get_resistance(v);
  for(int i=0; i < sizeof(wheel_ohm)/sizeof(wheel_ohm[0]); i++) {
    if(r > (wheel_ohm[i] - WHEEL_OHM_TOLERANCE) && r < (wheel_ohm[i] + WHEEL_OHM_TOLERANCE)) {
      return i;
    }
  }
  return WHEEL_NONE;
}

