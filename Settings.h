#define SETTINGSOPTIONSNO 4
#define SETTINGSVALUESNO 18//Maximum number of settings option values needed
int settingsValueIndex = 0;//currently selected settings option value index

struct SettingsOption
{
  char * option;//Settings option string
  char *value[SETTINGSVALUESNO];//Array of strings of settings option values
  int  handler;//Function to handle the values for this settings option
  int  currentIndex;//Function to array index of current value for this settings option
};

//void settingsMIDICh(char * value);
//void settingsMIDIOutCh(char * value);
void settingsEncoderDir(char * value);
void settingsPickupEnable(char * value);
void settingsDetune(char * value);
void settingsProgram(char * value);
//void settingsCCType(char * value);
void settingsHandler(char * s, void (*f)(char*));

//int currentIndexMIDICh();
//int currentIndexMIDIOutCh();
int currentIndexEncoderDir();
int currentIndexPickupEnable();
int currentIndexDetune();
int currentIndexProgram();
//int currentIndexCCType();
int getCurrentIndex(int (*f)());


void settingsMIDICh(char * value) {
  if (strcmp(value, "ALL") == 0) {
    midiChannel = MIDI_CHANNEL_OMNI;
  } else {
    midiChannel = atoi(value);
  }
  storeMidiChannel(midiChannel);
}

void settingsMIDIOutCh(char * value) {
  if (strcmp(value, "Off") == 0) {
    midiOutCh = 0;
  } else {
    midiOutCh = atoi(value);
  }
  storeMidiOutCh(midiOutCh);
}

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

//Takes a pointer to a specific method for the settings option and invokes it.
void settingsHandler(char * s, void (*f)(char*) ) {
  f(s);
}

int currentIndexMIDICh() {
  return getMIDIChannel();
}

int currentIndexMIDIOutCh() {
  return getMIDIOutCh();
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

//Takes a pointer to a specific method for the current settings option value and invokes it.
int getCurrentIndex(int (*f)() ) {
  return f();
}

CircularBuffer<SettingsOption, SETTINGSOPTIONSNO>  settingsOptions;

// add settings to the circular buffer
void setUpSettings() {
//  settingsOptions.push(SettingsOption{"MIDI Ch.", {"All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", '\0'}, settingsMIDICh, currentIndexMIDICh});
//  settingsOptions.push(SettingsOption{"MIDI Out Ch.", {"Off", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", '\0'}, settingsMIDIOutCh, currentIndexMIDIOutCh});
  settingsOptions.push(SettingsOption{"Encoder", {"Type 1", "Type 2", '\0'}, settingsEncoderDir, currentIndexEncoderDir});
  settingsOptions.push(SettingsOption{"Pick-up", {"Off", "On", '\0'}, settingsPickupEnable, currentIndexPickupEnable});
  settingsOptions.push(SettingsOption{"Unison Det", {"Off", "On", '\0'}, settingsDetune, currentIndexDetune});
  settingsOptions.push(SettingsOption{"PGM Change", {"Off", "On", '\0'}, settingsProgram, currentIndexProgram});
}
