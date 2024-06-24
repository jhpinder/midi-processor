#include <MIDI.h>
#include <RF24MIDI.h>

#define RF24MIDIINADDR  49
#define RF24MIDIOUTADDR 50

RF24MIDI_CREATE_INSTANCE(RF24MIDIINADDR, RF24MIDIOUTADDR, RF24MIDI);
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
   MIDI.begin(MIDI_CHANNEL_OMNI);
   MIDI.turnThruOff();
   RF24MIDI.begin(MIDI_CHANNEL_OMNI);
   RF24MIDI.turnThruOff();
}

void loop() {

  if (MIDI.read())
  {
     RF24MIDI.send(MIDI.getType(),
                   MIDI.getData1(),
                   MIDI.getData2(),
                   MIDI.getChannel());
  }
  if (RF24MIDI.read())
  {
     MIDI.send(RF24MIDI.getType(),
               RF24MIDI.getData1(),
               RF24MIDI.getData2(),
               RF24MIDI.getChannel());
  }
}
