#include <RF24MIDI.h>
#include <MIDI.h>

#define RF24INADDR   42
#define RF24OUTADDR  43

MIDI_CREATE_DEFAULT_INSTANCE();
RF24MIDI_CREATE_INSTANCE(RF24INADDR, RF24OUTADDR, RF24MIDI);

void setup() {
  // put your setup code here, to run once:
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  RF24MIDI.begin(MIDI_CHANNEL_OMNI);
  RF24MIDI.turnThruOff();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (RF24MIDI.read()) {
     MIDI.send(RF24MIDI.getType(),
               RF24MIDI.getData1(),
               RF24MIDI.getData2(),
               RF24MIDI.getChannel());
  }
}
