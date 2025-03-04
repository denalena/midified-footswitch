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

  buttons[0].pin = 9;
  buttons[1].pin = 8;
  buttons[2].pin = 7;
  buttons[3].pin = 6;
  
  buttons[0].cc = 64;
  buttons[1].cc = 65;
  buttons[2].cc = 66;
  buttons[3].cc = 67;

  for (int i=0; i<NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].state = digitalRead(buttons[i].pin);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  for (int i=0; i<NUM_BUTTONS; i++) {
    // read current value
    button& currentButton = buttons[i];
    bool value = digitalRead(currentButton.pin);

    unsigned long currentDelta = currentMillis - currentButton.changed;

    // skip to next button if state did not change slow enough
    if (
      value == currentButton.state
      || currentMillis - currentButton.changed < DEBOUNCE_MS
    ) {
      continue;
    }

    currentButton.oldState = currentButton.state;
    currentButton.state = !currentButton.state;
    currentButton.changed = currentMillis;

    unsigned short ccValue = currentButton.ccMin;

    if (currentButton.state) {
      ccValue = currentButton.ccMax;
    }

    sendMIDI(i, currentButton.ccChannel, currentButton.cc, ccValue);
  }

  MidiUSB.flush();
}

void sendMIDI(int buttonId, int channel, int cc, int value) {
    midiEventPacket_t event = {0x0B, 0xB0 | channel, cc, value};
    MidiUSB.sendMIDI(event);

    Serial.print("button ");
    Serial.print(buttonId);
    Serial.print(" event: ");
    Serial.print(event.byte1, HEX);
    Serial.print(" ");
    Serial.print(event.byte2, HEX);
    Serial.print(" ");
    Serial.println(event.byte3, HEX);
}
