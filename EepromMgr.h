#include <EEPROM.h>

#define EEPROM_ENCODER_DIR 1
#define EEPROM_VOICES 4
#define EEPROM_PICKUP_ENABLE 6
#define EEPROM_DETUNE 7
#define EEPROM_PROGRAM 8

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
  if (Detune < 0 || Detune > 1)return false; //If EEPROM has no pickup enable stored
  return Detune == 1 ? true : false;
}

void storeDetune(byte Detune)
{
  EEPROM.update(EEPROM_DETUNE, Detune);
}

boolean getProgram()
{
  byte Program = EEPROM.read(EEPROM_PROGRAM);
  if (Program < 0 || Program > 1)return false; //If EEPROM has no pickup enable stored
  return Program == 1 ? true : false;
}

void storeProgram(byte Program)
{
  EEPROM.update(EEPROM_PROGRAM, Program);
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
