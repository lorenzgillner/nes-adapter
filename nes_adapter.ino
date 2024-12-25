/**
 * Install the Joystick library first: https://github.com/MHeironimus/ArduinoJoystickLibrary
 * For pinout information, visit the NES wiki: https://www.nesdev.org/wiki/Controller_port_pinout
 */

/* depending on your board, adjust pins */
#define CLOCK_PIN 7
#define LATCH_PIN 8
#define DATA_PIN 9
#define RX_LED 17
#define TX_LED 30

#define TIMEOUT 10

#include <Joystick.h>

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   4, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering

enum button_names { b_A,
                    b_B,
                    b_select,
                    b_start,
                    b_up,
                    b_down,
                    b_left,
                    b_right };

byte lastControllerState = 0;

void setup() {
  /* set up board LEDs (for sanity checks) */
  pinMode(RX_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);

  /* clock pin (write) */
  pinMode(CLOCK_PIN, OUTPUT);

  /* latch pin (write) */
  pinMode(LATCH_PIN, OUTPUT);

  /* data pin (read) */
  pinMode(DATA_PIN, INPUT);

  /* initialize clock and latch */
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);

  /* set up the joystick interface */
  Joystick.begin();
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
}

void loop() {
  /* sanity check ... */
  digitalWrite(RX_LED, HIGH);
  digitalWrite(TX_LED, HIGH);

  /* read controller state */
  byte controllerState = readControllerState();

  /* light up LEDs if a button was pressed */
  if (controllerState > 0) {
    digitalWrite(RX_LED, LOW);
    digitalWrite(TX_LED, LOW);
  }

  /* dispatch button presses */
  for (int b_i = 0; b_i < 8; b_i++) {
    /* query the current button state */
    bool buttonState = bitRead(controllerState, b_i);

    /* update joypad state only if necessary */
    if (buttonState != bitRead(lastControllerState, b_i)) {
      switch (b_i) {
        case b_A:
        case b_B:
        case b_start:
        case b_select:
          Joystick.setButton(b_i, buttonState);
          break;
        case b_up:
          Joystick.setYAxis(-buttonState);
          break;
        case b_down:
          Joystick.setYAxis(buttonState);
          break;
        case b_left:
          Joystick.setXAxis(-buttonState);
          break;
        case b_right:
          Joystick.setXAxis(buttonState);
          break;
        default:
          break;
      }
      bitWrite(lastControllerState, b_i, buttonState);
    }
  }
  // delay(TIMEOUT);
}

/* send a short impulse to the specified output pin */
void trigger(int addr) {
  digitalWrite(addr, HIGH);
  digitalWrite(addr, LOW);
}

/* read the controller state, i.e. pressed buttons */
byte readControllerState() {
  byte controllerState = 0;

  /* trigger latch to capture controller state */
  trigger(LATCH_PIN);

  /* read out the shift register */
  for (int i = 0; i < 8; i++) {
    bitWrite(controllerState, i, (digitalRead(DATA_PIN) == LOW));
    trigger(CLOCK_PIN);
  }

  return controllerState;
}
