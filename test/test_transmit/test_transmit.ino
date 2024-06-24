#include <MIDI.h>
#include <RF24MIDI.h>
#define RF24INADDR   43
#define RF24OUTADDR  42
MIDI_CREATE_DEFAULT_INSTANCE();
//RF24MIDI_CREATE_INSTANCE(RF24INADDR, RF24OUTADDR, RF24MIDI);
void setup() {
  // put your setup code here, to run once:
//  RF24MIDI.begin(MIDI_CHANNEL_OMNI);
//  RF24MIDI.turnThruOff();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 60; i < 72; i++) {
      MIDI.sendNoteOn(i, 64, 1);
      delay(300);
      MIDI.sendNoteOff(i, 0, 1);
      delay(50);
  }
  delay(500);
    for (int i = 60; i < 72; i++) {
      MIDI.sendNoteOn(i, 64, 0);
      delay(300);
      MIDI.sendNoteOff(i, 0, 1);
      delay(50);
  }
  delay(500);
}
