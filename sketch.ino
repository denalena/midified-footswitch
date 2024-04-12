#include "MIDIUSB.h"

#define NUM_BUTTONS 4
#define DEBOUNCE_MS 20

struct button {
  bool state    = false;
  bool oldState = false;
  
  unsigned char pin;
  unsigned char cc        = 3; // undefined CC
  unsigned char ccMin     = 0;
  unsigned char ccMax     = 127;
  unsigned char ccChannel = 0;
  
  unsigned long changed = 0;
} buttons[NUM_BUTTONS];

void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("START");

  buttons[0].pin = 18;
  buttons[1].pin = 19;
  buttons[2].pin = 20;
  buttons[3].pin = 21;

  buttons[0].cc = 80;
  buttons[1].cc = 81;
  buttons[2].cc = 82;
  buttons[3].cc = 83;

  for (int i=0; i<NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  for (int i=0; i<NUM_BUTTONS; i++) {
    // read current value
    button& currentButton = buttons[i];
    bool value = digitalRead(currentButton.pin);

    // skip to next button if state did not change slow enough
    if (
      value == currentButton.state
      || currentMillis - currentButton.changed < DEBOUNCE_MS
    ) {
      continue;
    }

    currentButton.state = !currentButton.state;
    currentButton.changed = currentMillis;

    unsigned short ccValue = currentButton.ccMin;

    if (currentButton.state) {
      ccValue = currentButton.ccMax;
    }

    midiEventPacket_t event = {0x0B, 0xB0 | currentButton.ccChannel, currentButton.cc, ccValue};
    MidiUSB.sendMIDI(event);

    Serial.print("button ");
    Serial.print(i);
    Serial.print(" event: ");
    Serial.print(event.byte1, HEX);
    Serial.print(" ");
    Serial.print(event.byte2, HEX);
    Serial.print(" ");
    Serial.println(event.byte3, HEX);
  }

  MidiUSB.flush();
}
