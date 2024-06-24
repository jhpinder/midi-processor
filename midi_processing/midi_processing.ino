#include <MIDI.h>

#define SWELL_CHANNEL 1
#define GREAT_CHANNEL 2
#define PEDAL_CHANNEL 3
#define CHOIR_CHANNEL 4
#define SOLO_CHANNEL 5
#define PISTON_CHANNEL 8
#define CANCEL_BUTTON 0x7F

MIDI_CREATE_DEFAULT_INSTANCE();

byte pistonInputPins[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
byte currPistonState[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte prevPistonState[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
byte pistonLampPins[10] = {30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
byte cancelPistonPin = 12;
bool prevCancelState = false;

byte keyboardButtonBasePin = 20;
byte keyboardLedBasePin = 25;
byte keyboardButtonChannels[4] = {4, 2, 1, 5};
byte currKeyboardButtonState[4] = {LOW, LOW, LOW, LOW};
byte prevKeyboardButtonState[4] = {LOW, LOW, LOW, LOW};

byte togglePedalButton = 40;
byte pedalStateLED = 41;

bool activeNotes[128];
byte activeMidiChannel = 2;
byte previousMidiChannel = 2;
bool currPedalState = true;
bool prevPedalButtonState = false;
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
  for (int i = 0; i < 10; i++) {
    pinMode(pistonInputPins[i], INPUT_PULLUP);
    pinMode(pistonLampPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(keyboardButtonBasePin + i, INPUT_PULLUP);
    pinMode(keyboardLedBasePin + i, OUTPUT);
  }
  pinMode(togglePedalButton, INPUT_PULLUP);
  pinMode(pedalStateLED, OUTPUT);
  pinMode(cancelPistonPin, INPUT_PULLUP);
}

void loop() {
  scanPistons();
  scanButtons();
  MIDI.read();
  sendPedalNotes();
}


void handleNoteOn(byte channel, byte note, byte velocity) {
  activeNotes[note] = true;
  MIDI.sendNoteOn(note, velocity, activeMidiChannel);
  if (lowestNoteWasReleased) {
    lowestNoteWasReleased = note > lastPedalNote;
  }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  activeNotes[note] = false;
  MIDI.sendNoteOff(note, velocity, activeMidiChannel);
  lowestNoteWasReleased = (lastPedalNote == note) && (lowestNoteOn() > note);
}

void handleCC(byte channel, byte number, byte value) {
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

void scanPistons() {
  for (int i = 0; i < 10; i++) {
    currPistonState[i] = !digitalRead(pistonInputPins[i]);
    if (currPistonState[i] != prevPistonState[i]
    && currPistonState
    && millis() - prevMillis > 300) {
      prevMillis = millis();
      for (int j = 0; j < 10; j++) {
        digitalWrite(pistonLampPins[j], i == j);
      }
      MIDI.sendProgramChange(i, PISTON_CHANNEL);
    }
    prevPistonState[i] = currPistonState[i];
  }
}

void scanButtons() {
  for (int i = 0; i < 4; i++) {
    currKeyboardButtonState[i] = !digitalRead(i + keyboardButtonBasePin);
    if (currKeyboardButtonState[i] != prevKeyboardButtonState[i] && currKeyboardButtonState[i]) {
      activeMidiChannel = keyboardButtonChannels[i];
      if (currentPedalNote > -1) {
        for (int j = 0; j < 128; j++) {
          if (activeNotes[j]) MIDI.sendNoteOff(j, 0, previousMidiChannel);
        }
      }
    }
    digitalWrite(i + keyboardLedBasePin, activeMidiChannel == keyboardButtonChannels[i]);
  }

  if (!digitalRead(togglePedalButton)
    && !prevPedalButtonState
    && millis() - prevMillis > 100) {
    prevMillis = millis();
    currPedalState = !currPedalState;
    digitalWrite(pedalStateLED, currPedalState);
    prevPedalButtonState = true;
  } else {
    prevPedalButtonState = false;
  }

  if (!digitalRead(cancelPistonPin)
    && !prevCancelState
    && millis() - prevMillis > 100) {
    MIDI.sendProgramChange(CANCEL_BUTTON, PISTON_CHANNEL); // send cancel button
    prevMillis = millis();
    prevCancelState = true;
  } else {
    prevCancelState = false;
  }
}

