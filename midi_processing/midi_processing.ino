#include <MIDI.h>

#define SWELL_CHANNEL   1
#define GREAT_CHANNEL   2
#define PEDAL_CHANNEL   3
#define CHOIR_CHANNEL   4
#define SOLO_CHANNEL    5
#define PISTON_CHANNEL  8

#define LAUNCHKEY_PISTON_CHANNEL   10
#define LAUNCHKEY_CANCEL_NUM     0x72
#define LAUNCHKEY_BASS_ON        0x68
#define LAUNCHKEY_BASS_OFF       0x69
#define LAUNCHKEY_MIN_KEY        0x33
#define LAUNCHKEY_MAX_KEY        0x37
#define PISTON_PRG_BASE          0x28

#define CANCEL_BUTTON 0x7F


MIDI_CREATE_DEFAULT_INSTANCE();

byte keyboardButtonChannels[4] = {4, 2, 1, 5};

bool activeNotes[128];
byte activeMidiChannel = 2;
byte previousMidiChannel = 2;
bool currPedalState = true;
int currentPedalNote = -1;
int lastPedalNote = -1;
bool lowestNoteWasReleased = false;

byte currentKeyboardChannel = 1;

long prevMillis = 0;

void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleCC);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

void loop() {
  MIDI.read();
  sendPedalNotes();
}


void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel == LAUNCHKEY_PISTON_CHANNEL) {
    MIDI.sendProgramChange(note - PISTON_PRG_BASE, PISTON_CHANNEL);
    return;
  }
  activeNotes[note] = true;
  MIDI.sendNoteOn(note, velocity, activeMidiChannel);
  if (lowestNoteWasReleased) {
    lowestNoteWasReleased = note > lastPedalNote;
  }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  if (channel == LAUNCHKEY_PISTON_CHANNEL) {
    return;
  }
  activeNotes[note] = false;
  MIDI.sendNoteOff(note, velocity, activeMidiChannel);
  lowestNoteWasReleased = (lastPedalNote == note)
    && (lowestNoteOn() > note)
    && (autoPedalDisabled());
}

void handleCC(byte channel, byte number, byte value) {
  if (number == LAUNCHKEY_CANCEL_NUM) {
    if (value) {
      MIDI.sendProgramChange(CANCEL_BUTTON, PISTON_CHANNEL);
    }
    return;
  }

  if (number == LAUNCHKEY_BASS_ON) {
    if (value) {
      currPedalState = true;
      if (lowestNoteOn() > -1) {
        MIDI.sendNoteOn(lowestNoteOn(), 64, PEDAL_CHANNEL);
      }
    }
    return;
  }

  if (number == LAUNCHKEY_BASS_OFF) {
    if (value) {
      currPedalState = false;
      if (lowestNoteOn() > -1) {
        MIDI.sendNoteOff(currentPedalNote, 0, PEDAL_CHANNEL);
      }
    }
    return;
  }

  if (number >= LAUNCHKEY_MIN_KEY && number < LAUNCHKEY_MAX_KEY) {
    if (value) {
      activeMidiChannel = keyboardButtonChannels[number - LAUNCHKEY_MIN_KEY];
      for (int j = 0; j < 128; j++) {
        if (activeNotes[j]) {
          MIDI.sendNoteOn(j, 64, activeMidiChannel);
          MIDI.sendNoteOff(j, 0, previousMidiChannel);
        }
      }
      previousMidiChannel = activeMidiChannel;
    }
    return;
  }

  MIDI.sendControlChange(number, value, activeMidiChannel);
}

void sendPedalNotes() {
  currentPedalNote = lowestNoteOn();
  if (currentPedalNote == lastPedalNote) {
    return;
  }
  if (lastPedalNote >= 0) {
    MIDI.sendNoteOff(lastPedalNote, 0, PEDAL_CHANNEL);
  }
  lastPedalNote = currentPedalNote;

  if (currentPedalNote < 0) {
    return;
  }

  if (!currPedalState) {
    MIDI.sendNoteOff(currentPedalNote, 0, PEDAL_CHANNEL);
    return;
  }

  if (lowestNoteWasReleased) {
    return;
  }
  
  MIDI.sendNoteOn(currentPedalNote, 64, PEDAL_CHANNEL);

}

int lowestNoteOn() {
  for (int i = 0; i < 128; i++) {
    if (activeNotes[i]) {
      return i;
    }
  }
  return -1;
}

bool autoPedalDisabled() {
  byte numNotes = 0;
  byte largestGap = 0;
  byte bottom = -1;
  byte top = -1;
  for (int i = 0; i < 128; i++) {
    if (activeNotes[i]) {
      top = i;
      largestGap = top - bottom > largestGap && bottom >= 0 ? top - bottom : largestGap;
      numNotes++;;
      bottom = i;
    }
  }
  return numNotes > 2 && largestGap < 5;
}

