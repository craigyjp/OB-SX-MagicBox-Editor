#define SETTINGSOPTIONSNO 11
#define SETTINGSVALUESNO 18//Maximum number of settings option values needed
int settingsValueIndex = 0;//currently selected settings option value index

struct SettingsOption
{
  char * option;//Settings option string
  char *value[SETTINGSVALUESNO];//Array of strings of settings option values
  int  handler;//Function to handle the values for this settings option
  int  currentIndex;//Function to array index of current value for this settings option
};

void settingsEncoderDir(char * value);
void settingsPickupEnable(char * value);
void settingsDetune(char * value);
void settingsProgram(char * value);
void settingsNumberVoices(char * value);
//void settingsModAmount(char * value);
void settingsSaveMagicPatch(char * value);
void settingsAddingPatch(char * value);
void settingsUSBNote(char * value);
void settingsUSBBend(char * value);
void settingsUSBMod(char * value);
void settingsUSBChannel(char * value);
void settingsHandler(char * s, void (*f)(char*));

int currentIndexEncoderDir();
int currentIndexPickupEnable();
int currentIndexDetune();
int currentIndexProgram();
int currentIndexNumberVoices();
//int currentIndexModAmount();
int currentIndexSaveMagicPatch();
int currentIndexAddingPatch();
int currentIndexUSBChannel();
int currentIndexUSBNote();
int currentIndexUSBBend();
int currentIndexUSBMod();
int getCurrentIndex(int (*f)());

void settingsEncoderDir(char * value) {
  if (strcmp(value, "Type 1") == 0) {
    encCW = true;
  } else {
    encCW = false;
  }
  storeEncoderDir(encCW ? 1 : 0);
}

void settingsPickupEnable(char * value) {
  if (strcmp(value, "Off") == 0) {
    pickUpActive = false;
  } else {
    pickUpActive = true;
  }
  storePickupEnable(pickUpActive ? 1 : 0);
}

void settingsDetune(char * value) {
  if (strcmp(value, "Off") == 0) {
    Detune = false;
  } else {
    Detune = true;
  }
  storeDetune(Detune ? 1 : 0);
}

void settingsProgram(char * value) {
  if (strcmp(value, "Off") == 0) {
    Program = false;
  } else {
    Program = true;
  }
  storeProgram(Program ? 1 : 0);
}

void settingsNumberVoices(char * value) {
  Voices = atoi(value);
  storeNumberVoices(Voices);
}

void settingsModAmount(char * value) {
  if (strcmp(value, "Off") == 0) {
    modAmount = false;
  } else {
    modAmount = true;
  }
  storeModAmount(modAmount ? 1 : 0);
}

void settingsSaveMagicPatch(char * value) {
  if (strcmp(value, "Off") == 0) {
    saveMagicPatch = false;
  } else {
    saveMagicPatch = true;
  }
  storeSaveMagicPatch(saveMagicPatch ? 1 : 0);
}

void settingsAddingPatch(char * value) {
  if (strcmp(value, "Off") == 0) {
    addingPatch = false;
  } else {
    addingPatch = true;
  }
  storeAddingPatch(addingPatch ? 1 : 0);
}

void settingsUSBNote(char * value) {
  if (strcmp(value, "Off") == 0) {
    usbNote = false;
  } else {
    usbNote = true;
  }
  storeUSBNote(usbNote ? 1 : 0);
}

void settingsUSBBend(char * value) {
  if (strcmp(value, "Off") == 0) {
    usbBend = false;
  } else {
    usbBend = true;
  }
  storeUSBBend(usbBend ? 1 : 0);
}

void settingsUSBMod(char * value) {
  if (strcmp(value, "Off") == 0) {
    usbMod = false;
  } else {
    usbMod = true;
  }
  storeUSBMod(usbMod ? 1 : 0);
}

void settingsUSBChannel(char * value) {
  if (strcmp(value, "ALL") == 0) {
    usbChannel = MIDI_CHANNEL_OMNI;
  } else {
    usbChannel = atoi(value);
  }
  storeUSBChannel(usbChannel);
}

//Takes a pointer to a specific method for the settings option and invokes it.
void settingsHandler(char * s, void (*f)(char*) ) {
  f(s);
}

int currentIndexEncoderDir() {
  return getEncoderDir() ? 0 : 1;
}

int currentIndexPickupEnable() {
  return getPickupEnable() ? 1 : 0;
}

int currentIndexDetune() {
  return getDetune() ? 1 : 0;
}

int currentIndexProgram() {
  return getProgram() ? 1 : 0;
}

int currentIndexNumberVoices() {
  return getNumberVoices() -1;
}

int currentIndexModAmount() {
  return getModAmount() ? 1 : 0;
}

int currentIndexSaveMagicPatch() {
  return getSaveMagicPatch() ? 1 : 0;
}

int currentIndexAddingPatch() {
  return getAddingPatch() ? 1 : 0;
}

int currentIndexUSBNote() {
  return getUSBNote() ? 1 : 0;
}

int currentIndexUSBBend() {
  return getUSBBend() ? 1 : 0;
}

int currentIndexUSBMod() {
  return getUSBMod() ? 1 : 0;
}

int currentIndexUSBChannel() {
  return getUSBChannel();
}

//Takes a pointer to a specific method for the current settings option value and invokes it.
int getCurrentIndex(int (*f)() ) {
  return f();
}

CircularBuffer<SettingsOption, SETTINGSOPTIONSNO>  settingsOptions;

// add settings to the circular buffer
void setUpSettings() {
  settingsOptions.push(SettingsOption{"Encoder", {"Type 1", "Type 2", '\0'}, settingsEncoderDir, currentIndexEncoderDir});
  settingsOptions.push(SettingsOption{"Pick-up", {"Off", "On", '\0'}, settingsPickupEnable, currentIndexPickupEnable});
  settingsOptions.push(SettingsOption{"Unison Det", {"Off", "On", '\0'}, settingsDetune, currentIndexDetune});
  //settingsOptions.push(SettingsOption{"Mod Amount", {"Off", "On", '\0'}, settingsModAmount, currentIndexModAmount});
  settingsOptions.push(SettingsOption{"PGM Change", {"Off", "On", '\0'}, settingsProgram, currentIndexProgram});
  settingsOptions.push(SettingsOption{"Save Patch", {"Off", "On", '\0'}, settingsSaveMagicPatch, currentIndexSaveMagicPatch});
  settingsOptions.push(SettingsOption{"Add Patches", {"Off", "On", '\0'}, settingsAddingPatch, currentIndexAddingPatch});
  settingsOptions.push(SettingsOption{"USB Notes", {"Off", "On", '\0'}, settingsUSBNote, currentIndexUSBNote});
  settingsOptions.push(SettingsOption{"USB Bend", {"Off", "On", '\0'}, settingsUSBBend, currentIndexUSBBend});
  settingsOptions.push(SettingsOption{"USB Mod", {"Off", "On", '\0'}, settingsUSBMod, currentIndexUSBMod});
  settingsOptions.push(SettingsOption{"USB Ch.", {"All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", '\0'}, settingsUSBChannel, currentIndexUSBChannel});
  settingsOptions.push(SettingsOption{"Voices", {"1", "2", "3", "4", "5", "6", '\0'}, settingsNumberVoices, currentIndexNumberVoices});
}
