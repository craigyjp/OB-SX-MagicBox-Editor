#define SETTINGSOPTIONSNO 5
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
void settingsHandler(char * s, void (*f)(char*));

int currentIndexEncoderDir();
int currentIndexPickupEnable();
int currentIndexDetune();
int currentIndexProgram();
int currentIndexNumberVoices();
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
  settingsOptions.push(SettingsOption{"PGM Change", {"Off", "On", '\0'}, settingsProgram, currentIndexProgram});
  settingsOptions.push(SettingsOption{"Voices", {"1", "2", "3", "4", "5", "6", '\0'}, settingsNumberVoices, currentIndexNumberVoices});
}
