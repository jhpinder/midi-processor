#include <MIDI.h>

#define SWELL_CHANNEL 1
#define GREAT_CHANNEL 2
#define PEDAL_CHANNEL 3
#define CHOIR_CHANNEL 4
#define SOLO_CHANNEL 5
#define PISTON_CHANNEL 8

MIDI_CREATE_DEFAULT_INSTANCE();

byte pistonInputPins[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
byte currPistonState[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte prevPistonState[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte pistonLampPins[10] = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
byte cancelPistonPin = 12;

byte keyboardButtonBasePin = 20;
byte keyboardButtonChannels[4] = {4, 2, 1, 5};
byte currKeyboardButtonState[4] = {LOW, LOW, LOW, LOW};
byte prevKeyboardButtonState[4] = {LOW, LOW, LOW, LOW};

byte togglePedalButton = 40;
byte pedalStateLED = 41;

bool activeNotes[128];
byte activeMidiChannel = 2;
byte previousMidiChannel = 2;
bool currPedalActive = true;
bool prevPedalActive = true;
int currentPedalNote = -1;
int lastPedalNote = -1;

byte currentKeyboardChannel = 1;

long prevMillis = 0;

void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.turnThruOff();
  MIDI.begin();
  for (int i = 0; i < 10; i++) {
    pinMode(pistonInputPins[i], INPUT_PULLUP);
    pinMode(pistonLampPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(keyboardButtonBasePin + i, INPUT_PULLUP);
  }
  pinMode(togglePedalButton, INPUT_PULLUP);
  pinMode(pedalStateLED, OUTPUT);


}

void loop() {
  scanPistons();
  scanButtons();
  MIDI.read();
  sendPedalNotes();
}


void handleNoteOn(byte channel, byte note, byte velocity) {
  activeNotes[note] = true;
  MIDI.sendNoteOn(activeMidiChannel, note, velocity);
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  activeNotes[note] = false;
  MIDI.sendNoteOff(activeMidiChannel, note, velocity);
}

void handleCC(byte channel, byte number, byte value) {
  MIDI.sendControlChange(activeMidiChannel, number, value);
}

void sendPedalNotes() {
  currentPedalNote = lowestNoteOn();
  if (currentPedalNote == lastPedalNote) {
    return;
  }
  if (lastPedalNote >= 0) {
    MIDI.sendNoteOff(PEDAL_CHANNEL, lastPedalNote, 0);
  }
  lastPedalNote = currentPedalNote;
  
  if (currentPedalNote < 0) {
    return;
  }

  if (!currPedalActive) {
    return;
  }
  
  MIDI.sendNoteOn(PEDAL_CHANNEL, currentPedalNote, 127);
}

int lowestNoteOn() {
  for (int i = 0; i < 128; i++) {
    if (activeNotes[i]) {
      return i;
    }
  }
  return -1;
}

void scanPistons() {
  for (int i = 0; i < 10; i++) {
    currPistonState[i] = !digitalRead(pistonInputPins[i]);
    if (currPistonState[i] != prevPistonState[i] && currPistonState) {
      for (int j = 0; j < 10; j++) {
        digitalWrite(pistonLampPins[j], i == j);
      }
      MIDI.sendProgramChange(PISTON_CHANNEL, 1);
    }
  }
}

void scanButtons() {
  for (int i = 0; i < 4; i++) {
        currKeyboardButtonState[i] = !digitalRead(i + keyboardButtonBasePin);
    if (currKeyboardButtonState[i] != prevKeyboardButtonState[i] && currKeyboardButtonState[i]) {
      activeMidiChannel = keyboardButtonChannels[i];
      if (currentPedalNote > -1) {
        for (int j = 0; j < 128; j++) {
          if (activeNotes[j])
            MIDI.sendNoteOff(previousMidiChannel, j, 0);
        }
      }
    }    
  }
  if (!digitalRead(togglePedalButton)
    && currPedalActive != prevPedalActive
    && millis() - prevMillis > 20) {
    prevMillis = millis();
    currPedalActive = !currPedalActive;
    digitalWrite(pedalStateLED, currPedalActive);
    prevPedalActive = currPedalActive;
  }

  if (!digitalRead(cancelPistonPin) && millis() - prevMillis > 100) {
    MIDI.sendProgramChange(PISTON_CHANNEL, 1); // send cancel button
    prevMillis = millis();
  }
}

