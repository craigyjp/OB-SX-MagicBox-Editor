#include <EEPROM.h>

#define EEPROM_ENCODER_DIR 1
#define EEPROM_SAVE_PATCH 2
#define EEPROM_ADD_PATCH 3
#define EEPROM_VOICES 4
#define EEPROM_MOD_AMOUNT 5
#define EEPROM_PICKUP_ENABLE 6
#define EEPROM_DETUNE 7
#define EEPROM_PROGRAM 8
#define EEPROM_USB_NOTE 9
#define EEPROM_USB_BEND 10
#define EEPROM_USB_MOD 11
#define EEPROM_USB_CHANNEL 12

boolean getEncoderDir()
{
  byte ed = EEPROM.read(EEPROM_ENCODER_DIR);
  if (ed < 0 || ed > 1)return true; //If EEPROM has no encoder direction stored
  return ed == 1 ? true : false;
}

void storeEncoderDir(byte encoderDir)
{
  EEPROM.update(EEPROM_ENCODER_DIR, encoderDir);
}

boolean getPickupEnable()
{
  byte pu = EEPROM.read(EEPROM_PICKUP_ENABLE);
  if (pu < 0 || pu > 1)return false; //If EEPROM has no pickup enable stored
  return pu == 1 ? true : false;
}

void storePickupEnable(byte pickUpActive)
{
  EEPROM.update(EEPROM_PICKUP_ENABLE, pickUpActive);
}

boolean getDetune()
{
  byte Detune = EEPROM.read(EEPROM_DETUNE);
  if (Detune < 0 || Detune > 1)return false; //If EEPROM has no detune enable stored
  return Detune == 1 ? true : false;
}

void storeDetune(byte Detune)
{
  EEPROM.update(EEPROM_DETUNE, Detune);
}

boolean getProgram()
{
  byte Program = EEPROM.read(EEPROM_PROGRAM);
  if (Program < 0 || Program > 1)return false; //If EEPROM has no program enable stored
  return Program == 1 ? true : false;
}

void storeProgram(byte Program)
{
  EEPROM.update(EEPROM_PROGRAM, Program);
}

boolean getSaveMagicPatch()
{
  byte saveMagicPatch = EEPROM.read(EEPROM_SAVE_PATCH);
  if (saveMagicPatch < 0 || saveMagicPatch > 1)return false; //If EEPROM has no program enable stored
  return saveMagicPatch == 1 ? true : false;
}

void storeSaveMagicPatch(byte saveMagicPatch)
{
  EEPROM.update(EEPROM_SAVE_PATCH, saveMagicPatch);
}

boolean getAddingPatch()
{
  byte addingPatch = EEPROM.read(EEPROM_ADD_PATCH);
  if (addingPatch < 0 || addingPatch > 1)return false; //If EEPROM has no program enable stored
  return addingPatch == 1 ? true : false;
}

void storeAddingPatch(byte addingPatch)
{
  EEPROM.update(EEPROM_ADD_PATCH, addingPatch);
}

boolean getModAmount()
{
  byte modAmount = EEPROM.read(EEPROM_MOD_AMOUNT);
  if (modAmount < 0 || modAmount > 1)return false; //If EEPROM has no modAmount enable stored
  return modAmount == 1 ? true : false;
}

void storeModAmount(byte modAmount)
{
  EEPROM.update(EEPROM_MOD_AMOUNT, modAmount);
}

int getNumberVoices() {
  byte Voices = EEPROM.read(EEPROM_VOICES);
  if (Voices < 1 || Voices > 6) return 6;//If EEPROM has no Voices stored
  return Voices;
}

void storeNumberVoices(byte Voices)
{
  EEPROM.update(EEPROM_VOICES, Voices);
}

void storeUSBNote(byte usbNote)
{
  EEPROM.update(EEPROM_USB_NOTE, usbNote);
}

int getUSBNote() {
  byte usbNote = EEPROM.read(EEPROM_USB_NOTE);
  if (usbNote < 0 || usbNote > 1) return false;//If EEPROM has no Voices stored
  return usbNote == 1 ? true : false;
}

void storeUSBBend(byte usbBend)
{
  EEPROM.update(EEPROM_USB_BEND, usbBend);
}

int getUSBBend() {
  byte usbBend = EEPROM.read(EEPROM_USB_BEND);
  if (usbBend < 0 || usbBend > 1) return false;//If EEPROM has no Voices stored
  return usbBend == 1 ? true : false;
}

void storeUSBMod(byte usbMod)
{
  EEPROM.update(EEPROM_USB_MOD, usbMod);
}

int getUSBMod() {
  byte usbMod = EEPROM.read(EEPROM_USB_MOD);
  if (usbMod < 0 || usbMod > 1) return false;//If EEPROM has no Voices stored
  return usbMod == 1 ? true : false;
}

int getUSBChannel() {
  byte usbChannel = EEPROM.read(EEPROM_USB_CHANNEL);
  if (usbChannel < 0 || usbChannel > 16) usbChannel = 1;//If EEPROM has no MIDI channel stored
  return usbChannel;
}

void storeUSBChannel(byte usbChannel)
{
  EEPROM.update(EEPROM_USB_CHANNEL, usbChannel);
}
