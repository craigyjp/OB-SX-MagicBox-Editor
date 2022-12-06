#include <EEPROM.h>

#define EEPROM_MIDI_CH 0
#define EEPROM_ENCODER_DIR 1
#define EEPROM_LAST_PATCH 2
#define EEPROM_MIDI_OUT_CH 3
#define EEPROM_PICKUP_ENABLE 6
#define EEPROM_DETUNE 7
#define EEPROM_PROGRAM 8

int getMIDIChannel() {
  byte midiChannel = EEPROM.read(EEPROM_MIDI_CH);
  if (midiChannel < 0 || midiChannel > 16) midiChannel = MIDI_CHANNEL_OMNI;//If EEPROM has no MIDI channel stored
  return midiChannel;
}

void storeMidiChannel(byte channel)
{
  EEPROM.update(EEPROM_MIDI_CH, channel);
}

boolean getEncoderDir() {
  byte ed = EEPROM.read(EEPROM_ENCODER_DIR); 
  if (ed < 0 || ed > 1)return true; //If EEPROM has no encoder direction stored
  return ed == 1 ? true : false;
}

void storeEncoderDir(byte encoderDir)
{
  EEPROM.update(EEPROM_ENCODER_DIR, encoderDir);
}

boolean getPickupEnable() {
  byte pu = EEPROM.read(EEPROM_PICKUP_ENABLE);
  if (pu < 0 || pu > 1)return false; //If EEPROM has no pickup enable stored
  return pu == 1 ? true : false;
}

void storePickupEnable(byte pickUpActive) {
  EEPROM.update(EEPROM_PICKUP_ENABLE, pickUpActive);
}

boolean getDetune() {
  byte Detune = EEPROM.read(EEPROM_DETUNE);
  if (Detune < 0 || Detune > 1)return false; //If EEPROM has no pickup enable stored
  return Detune == 1 ? true : false;
}

void storeDetune(byte Detune) {
  EEPROM.update(EEPROM_DETUNE, Detune);
}

boolean getProgram() {
  byte Program = EEPROM.read(EEPROM_PROGRAM);
  if (Program < 0 || Program > 1)return false; //If EEPROM has no pickup enable stored
  return Program == 1 ? true : false;
}

void storeProgram(byte Program) {
  EEPROM.update(EEPROM_PROGRAM, Program);
}

int getLastPatch() {
  int lastPatchNumber = EEPROM.read(EEPROM_LAST_PATCH);
  if (lastPatchNumber < 1 || lastPatchNumber > 999) lastPatchNumber = 1;
  return lastPatchNumber;
}

void storeLastPatch(int lastPatchNumber)
{
  EEPROM.update(EEPROM_LAST_PATCH, lastPatchNumber);
}

int getMIDIOutCh() {
  byte mc = EEPROM.read(EEPROM_MIDI_OUT_CH);
  if (mc < 0 || midiOutCh > 16) mc = 0;//If EEPROM has no MIDI channel stored
  return mc;
}

void storeMidiOutCh(byte midiOutCh){
  EEPROM.update(EEPROM_MIDI_OUT_CH, midiOutCh);
}
