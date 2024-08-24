//Values below are just for initialising and will be changed when synth is initialised to current panel controls & EEPROM settings
byte midiChannel = 1;
byte usbChannel = 1;
boolean usbNote = false;
boolean usbBend = false;
boolean usbMod = false;
byte ccChannel = 1;
byte midiOutCh = 1;
byte Voices, oldVoices;
int readresdivider = 2;
boolean patchRecall = false;

#define TOLERANCE 3

int MIDIThru = midi::Thru::Off;//(EEPROM)
String patchName = INITPATCHNAME;
boolean encCW = true;//This is to set the encoder to increment when turned CW - Settings Option

int vcf_attack = 0;
int vca_attack = 0;
int vcf_decay = 0;
int vca_release = 0;
int vcf_frequency = 0;
int osc_2_detune = 0;
int lfo_frequency = 0;
int portamento = 0;
int vca_decay = 0;
int vcf_sustain = 0;
int vca_sustain = 0;
int vcf_release = 0;
int vcf_resonance = 0;
int vcf_resonancestr = 0;
int vcf_env_amount = 0;
int pulsewidth = 0;
int modulation = 0;
int oscpwm = 0;
int osc_2_freq = 0;
int osc_1_freq = 0;

int osc_2_fm = 0;
int vcf_mod = 0;
int osc_1_wave = 0;
int osc_2_wave = 0;
int lfo_wave = 0;
int osc_1_fm = 0;
int osc_1_pwm = 0;
int osc_2_pwm = 0;
int vcf_track = 0;
int osc_2_half = 0;
int sync = 0;
int crossmod = 0;
int unison = 0;

int voice1_detune = 0;
int voice2_detune = 0;
int voice3_detune = 0;
int voice4_detune = 0;
int voice5_detune = 0;
int voice6_detune = 0;

int midiModAmount = 0;

int vcf_attackPrevValue = 0; //Need to set these when patch loaded
int vca_attackPrevValue = 0; //Need to set these when patch loaded
int vcf_decayPrevValue = 0; //Need to set these when patch loaded
int vca_releasePrevValue = 0; //Need to set these when patch loaded
int vcf_frequencyPrevValue = 0; //Need to set these when patch loaded
int osc_2_detunePrevValue = 0; //Need to set these when patch loaded
int lfo_frequencyPrevValue = 0; //Need to set these when patch loaded
int portamentoPrevValue = 0; //Need to set these when patch loaded
int vca_decayPrevValue = 0; //Need to set these when patch loaded
int vcf_sustainPrevValue = 0; //Need to set these when patch loaded
int vca_sustainPrevValue = 0; //Need to set these when patch loaded
int vcf_releasePrevValue = 0; //Need to set these when patch loaded
int vcf_resonancePrevValue = 0; //Need to set these when patch loaded
int vcf_env_amountPrevValue = 0; //Need to set these when patch loaded
int pulsewidthPrevValue = 0; //Need to set these when patch loaded
int modulationPrevValue = 0; //Need to set these when patch loaded
int oscpwmPrevValue = 0; //Need to set these when patch loaded
int osc_1_freqPrevValue = 0; //Need to set these when patch loaded
int osc_2_freqPrevValue = 0; //Need to set these when patch loaded

int returnvalue = 0;

boolean pickUp = true;//settings option (EEPROM)
boolean pickUpActive = false;
boolean Detune = false;
boolean modAmount = false;
boolean saveMagicPatch = false;
boolean addingPatch = false;
boolean oldpickUpActive;
boolean Program = false;
boolean vcf_attackpickUp = true;
boolean vca_attackpickUp = true;
boolean vcf_decaypickUp = true;
boolean vca_releasepickUp = true;
boolean vcf_frequencypickUp = true;
boolean osc_2_detunepickUp = true;
boolean lfo_frequencypickUp = true;
boolean portamentopickUp = true;
boolean vca_decaypickUp = true;
boolean vcf_sustainpickUp = true;
boolean vca_sustainpickUp = true;
boolean vcf_releasepickUp = true;
boolean vcf_resonancepickUp = true;
boolean vcf_env_amountpickUp = true;
boolean pulsewidthpickUp = true;
boolean modulationpickUp = true;
boolean oscpwmpickUp = true;
boolean osc_1_freqpickUp = true;
boolean osc_2_freqpickUp = true;
