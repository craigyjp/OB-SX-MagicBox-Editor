/*
  OB-SX Editor - Firmware Rev 1.0

  Includes code by:
    Dave Benn - Handling MUXs, a few other bits and original inspiration  https://www.notesandvolts.com/2019/01/teensy-synth-part-10-hardware.html

  Arduino IDE
  Tools Settings:
  Board: "Teensy4,1"
  USB Type: "Serial + MIDI"
  CPU Speed: "600"
  Optimize: "Fastest"

  Additional libraries:
    Agileware CircularBuffer available in Arduino libraries manager
    Replacement files are in the Modified Libraries folder and need to be placed in the teensy Audio folder.
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MIDI.h>
#include <USBHost_t36.h>
#include "MidiCC.h"
#include "Constants.h"
#include "Parameters.h"
#include "PatchMgr.h"
#include "HWControls.h"
#include "EepromMgr.h"
#include <RoxMux.h>


#define PARAMETER 0 //The main page for displaying the current patch and control (parameter) changes
#define RECALL 1 //Patches list
#define SAVE 2 //Save patch page
#define REINITIALISE 3 // Reinitialise message
#define PATCH 4 // Show current patch bypassing PARAMETER
#define PATCHNAMING 5 // Patch naming page
#define DELETE 6 //Delete patch page
#define DELETEMSG 7 //Delete patch message page
#define SETTINGS 8 //Settings page
#define SETTINGSVALUE 9 //Settings page

unsigned int state = PARAMETER;

#include "ST7735Display.h"

boolean cardStatus = false;

//MIDI 5 Pin DIN
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
unsigned long buttonDebounce = 0;

#define OCTO_TOTAL 2
#define BTN_DEBOUNCE 50
RoxOctoswitch <OCTO_TOTAL, BTN_DEBOUNCE> octoswitch;

// pins for 74HC165
#define PIN_DATA  23 // pin 9 on 74HC165 (DATA)
#define PIN_LOAD  21 // pin 1 on 74HC165 (LOAD)
#define PIN_CLK   22 // pin 2 on 74HC165 (CLK))

#define SR_TOTAL 2
Rox74HC595 <SR_TOTAL> sr;

// pins for 74HC595
#define LED_DATA    6 // pin 14 on 74HC595 (DATA)
#define LED_LATCH   8  // pin 12 on 74HC595 (LATCH)
#define LED_CLK     7  // pin 11 on 74HC595 (CLK)
#define LED_PWM     -1  // pin 13 on 74HC595

#include "Settings.h"

int patchNo = 1;//Current patch no
int voiceToReturn = -1; //Initialise
long earliestTime = millis(); //For voice allocation - initialise to now

void setup()
{
  SPI.begin();
  octoswitch.begin(PIN_DATA, PIN_LOAD, PIN_CLK);
  octoswitch.setCallback(onButtonPress);
  sr.begin(LED_DATA, LED_LATCH, LED_CLK, LED_PWM);
  setupDisplay();
  setUpSettings();
  setupHardware();

  cardStatus = SD.begin(BUILTIN_SDCARD);
  if (cardStatus)
  {
    Serial.println("SD card is connected");
    //Get patch numbers and names from SD card
    loadPatches();
    if (patches.size() == 0)
    {
      //save an initialised patch to SD card
      savePatch("1", INITPATCH);
      loadPatches();
    }
  }
  else
  {
    Serial.println("SD card is not connected or unusable");
    reinitialiseToPanel();
    showPatchPage("No SD", "conn'd / usable");
  }


  //USB Client MIDI
  usbMIDI.setHandleControlChange(myMIDIChange);
  usbMIDI.setHandleProgramChange(myProgramChange);
  Serial.println("USB Client MIDI Listening");

  //MIDI 5 Pin DIN
  MIDI.begin();
  MIDI.setHandleControlChange(myMIDIChange);
  MIDI.setHandleProgramChange(myProgramChange);
  Serial.println("MIDI In DIN Listening");

  encCW = getEncoderDir();
  pickUpActive = getPickupEnable();
  oldpickUpActive = pickUpActive;
  Detune = getDetune();
  Program = getProgram();
  Voices = getNumberVoices();
  oldVoices = Voices;
  midiCCOut(120, Voices, 16);
  recallPatch(patchNo); //Load first patch
}

void allNotesOff()
{
}

void updatevcf_attack()
{
  showCurrentParameterPage("VCF Attack", String(vcf_attack), FILTER_ENV);
  midiCCOut((CCvcf_attack), vcf_attack, 1);
}

void updatevca_attack()
{
  showCurrentParameterPage("VCA Attack", String(vca_attack), AMP_ENV);
  midiCCOut((CCvca_attack), vca_attack, 1);
}

void updatevcf_decay()
{
  showCurrentParameterPage("VCF Decay", String(vcf_decay), FILTER_ENV);
  midiCCOut((CCvcf_decay), vcf_decay, 1);
}

void updatevca_release()
{
  showCurrentParameterPage("VCA Release", String(vca_release), AMP_ENV);
  midiCCOut((CCvca_release), vca_release, 1);
}

void updatevcf_frequency()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 5 Detune", String(voice5_detune));
    midiCCOut((108), voice5_detune, 5);
  }
  else
  {
    showCurrentParameterPage("VCF Freq", String(vcf_frequency));
    midiCCOut((CCvcf_frequency), vcf_frequency, 1);
  }
}

void updateosc_2_detune()
{
  showCurrentParameterPage("OSC 2 Detune", String(osc_2_detune));
  midiCCOut((CCosc_2_detune), osc_2_detune, 1);
}

void updatelfo_frequency()
{
  showCurrentParameterPage("LFO Rate", String(lfo_frequency));
  midiCCOut((CClfo_frequency), lfo_frequency, 1);
}

void updateportamento()
{
  showCurrentParameterPage("Portamento", String(portamento));
  midiCCOut((CCportamento), portamento, 1);
}

void updatevca_decay()
{
  showCurrentParameterPage("VCA Decay", String(vca_decay), AMP_ENV);
  midiCCOut((CCvca_decay), vca_decay, 1);
}

void updatevcf_sustain()
{
  showCurrentParameterPage("VCF Sustain", String(vcf_sustain), FILTER_ENV);
  midiCCOut((CCvcf_sustain), vcf_sustain, 1);
}

void updatevca_sustain()
{
  showCurrentParameterPage("VCA Sustain", String(vca_sustain), AMP_ENV);
  midiCCOut((CCvca_sustain), vca_sustain, 1);
}

void updatevcf_release()
{
  showCurrentParameterPage("VCF Release", String(vcf_release), FILTER_ENV);
  midiCCOut((CCvcf_release), vcf_release, 1);
}

void updatevcf_resonance()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 6 Detune", String(voice6_detune));
    midiCCOut((108), voice6_detune, 6);
  }
  else
  {
    showCurrentParameterPage("Resonance", String(vcf_resonance));
    midiCCOut((CCvcf_resonance), LINEAR_INVERSE[vcf_resonance], 1);
  }
}

void updatevcf_env_amount()
{
  showCurrentParameterPage("Env Amount", String(vcf_env_amount));
  midiCCOut((CCvcf_env_amount), vcf_env_amount, 1);
}

void updatepulsewidth()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 2 Detune", String(voice2_detune));
    midiCCOut((108), voice2_detune, 2);
  }
  else
  {
    showCurrentParameterPage("Pulse Width", String(pulsewidth));
    midiCCOut((CCpulsewidth), pulsewidth, 1);
  }
}

void updatemodulation()
{
  showCurrentParameterPage("Modulation", String(modulation));
  midiCCOut((CCmodulation), modulation, 1);
}

void updateoscpwm ()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 3 Detune", String(voice3_detune));
    midiCCOut((108), voice3_detune, 3);
  }
  else
  {
    showCurrentParameterPage("PWM", String(oscpwm));
    midiCCOut((CCoscpwm), oscpwm, 1);
  }
}

void updateosc_2_freq()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 4 Detune", String(voice4_detune));
    midiCCOut((108), voice4_detune, 4);
  }
  else
  {
    showCurrentParameterPage("OSC 2 Freq", String(osc_2_freq));
    midiCCOut((CCosc_2_freq), osc_2_freq, 1);
  }
}

void updateosc_1_freq()
{
  if (Detune == true)
  {
    showCurrentParameterPage("Osc 1 Detune", String(voice1_detune));
    midiCCOut((108), voice1_detune, 1);
  }
  else
  {
    showCurrentParameterPage("OSC 1 Freq", String(osc_1_freq));
    midiCCOut((CCosc_1_freq), osc_1_freq, 1);
  }
}

void updateosc_2_fm()
{
  if (osc_2_fm == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Mod", "On");
    }
    sr.writePin(OSC_2_MOD_LED, HIGH);  // LED on
    midiCCOut((CCosc_2_fm - 5), 127, 1);

  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Mod", "Off");
    }
    sr.writePin(OSC_2_MOD_LED, LOW);  // LED off
    midiCCOut((CCosc_2_fm - 5), 0, 1);
  }
}

void updatevcf_mod()
{
  if (vcf_mod == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("VCF Mod", "On");
    }
    sr.writePin(VCF_MOD_LED, HIGH);  // LED on
    midiCCOut((CCvcf_mod - 6), 127, 2);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("VCF Mod", "Off");
    }
    sr.writePin(VCF_MOD_LED, LOW);  // LED off
    midiCCOut((CCvcf_mod - 6), 0, 2);
  }
}

void updateosc_1_wave()
{
  if (osc_1_wave == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 Wave", "Pulse");
    }
    sr.writePin(OSC_1_WAVE_LED, HIGH);  // LED on
    midiCCOut((CCosc_1_wave - 7), 127, 3);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 Wave", "Sawtooth");
    }
    sr.writePin(OSC_1_WAVE_LED, LOW);  // LED off
    midiCCOut((CCosc_1_wave - 7), 0, 3);
  }
}

void updateosc_2_wave()
{
  if (osc_2_wave == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Wave", "Pulse");
    }
    sr.writePin(OSC_2_WAVE_LED, HIGH);  // LED on
    midiCCOut((CCosc_2_wave - 8), 127, 4);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Wave", "Sawtooth");
    }
    sr.writePin(OSC_2_WAVE_LED, LOW);  // LED off
    midiCCOut((CCosc_2_wave - 8), 0, 4);
  }
}

void updatelfo_wave()
{
  if (lfo_wave == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("LFO Wave", "Square");
    }
    sr.writePin(LFO_WAVE_LED, HIGH);  // LED on
    midiCCOut((CClfo_wave - 9), 127, 5);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("LFO Wave", "Triangle");
    }
    sr.writePin(LFO_WAVE_LED, LOW);  // LED off
    midiCCOut((CClfo_wave - 9), 0, 5);
  }
}

void updateosc_1_fm()
{
  if (osc_1_fm == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 Mod", "On");
    }
    sr.writePin(OSC_1_MOD_LED, HIGH);  // LED on
    midiCCOut((CCosc_1_fm - 10), 127, 6);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 Mod", "Off");
    }
    sr.writePin(OSC_1_MOD_LED, LOW);  // LED off
    midiCCOut((CCosc_1_fm - 10), 0, 6);
  }
}

void updateosc_1_pwm()
{
  if (osc_1_pwm == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 PWM", "On");
    }
    sr.writePin(OSC_1_PWM_LED, HIGH);  // LED on
    midiCCOut((CCosc_1_pwm - 14), 127, 1);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 1 PWM", "Off");
    }
    sr.writePin(OSC_1_PWM_LED, LOW);  // LED off
    midiCCOut((CCosc_1_pwm - 14), 0, 1);
  }
}

void updateosc_2_pwm()
{
  if (osc_2_pwm == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 PWM", "On");
    }
    sr.writePin(OSC_2_PWM_LED, HIGH);  // LED on
    midiCCOut((CCosc_2_pwm - 15), 127, 2);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 PWM", "Off");
    }
    sr.writePin(OSC_2_PWM_LED, LOW);  // LED off
    midiCCOut((CCosc_2_pwm - 15), 0, 2);
  }
}

void updatevcf_track()
{
  if (vcf_track == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("KBD Track", "On");
    }
    sr.writePin(KEYTRACK_LED, HIGH);  // LED on
    midiCCOut((CCvcf_track - 16), 127, 3);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("KBD Track", "Off");
    }
    sr.writePin(KEYTRACK_LED, LOW);  // LED off
    midiCCOut((CCvcf_track - 16), 0, 3);
  }
}

void updateosc_2_half()
{
  if (osc_2_half == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Half", "On");
    }
    sr.writePin(OSC_2_HALF_LED, HIGH);  // LED on
    midiCCOut((CCosc_2_half - 17), 127, 4);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Osc 2 Half", "Off");
    }
    sr.writePin(OSC_2_HALF_LED, LOW);  // LED off
    midiCCOut((CCosc_2_half - 17), 0, 4);
  }
}

void updatesync()
{
  if (sync == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Sync", "On");
    }
    sr.writePin(SYNC_LED, HIGH);  // LED on
    midiCCOut((CCsync - 18), 127, 5);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Sync", "Off");
    }
    sr.writePin(SYNC_LED, LOW);  // LED off
    midiCCOut((CCsync - 18), 0, 5);
  }
}

void updatecrossmod()
{
  if (crossmod == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("X-Mod", "On");
    }
    sr.writePin(CROSSMOD_LED, HIGH);  // LED on
    midiCCOut((CCcrossmod - 19), 127, 6);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("X-Mod", "Off");
    }
    sr.writePin(CROSSMOD_LED, LOW);  // LED off
    midiCCOut((CCcrossmod - 19), 0, 6);
  }
}


void updateUnison()
{
  if (unison == 127)
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Unison", "On");
    }
    sr.writePin(UNISON_LED, HIGH);  // LED on
    midiCCOut((CCunison - 20), 127, 7);
  }
  else
  {
    if (!patchRecall)
    {
      showCurrentParameterPage("Unison", "Off");
    }
    sr.writePin(UNISON_LED, LOW);  // LED off
    midiCCOut((CCunison - 20), 0, 7);
  }
}

void updatePatchname()
{
  showPatchPage(String(patchNo), patchName);
}

void myMIDIChange(byte channel, byte control, int value)
{
  value = (value * 2);
  myControlChange(channel, control, value);
}

void myControlChange(byte channel, byte control, int value)
{
  switch (control)
  {
    case CCvcf_attack:
      if (pickUpActive && vcf_attackpickUp && ((vcf_attackPrevValue + TOLERANCE) <  (value) || (vcf_attackPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vcf_attackpickUp = false;
      vcf_attack = value / readresdivider;
      updatevcf_attack();
      vcf_attackPrevValue = vcf_attack; //PICK-UP
      break;

    case CCvca_attack:
      if (pickUpActive && vca_attackpickUp && ((vca_attackPrevValue + TOLERANCE) <  (value) || (vca_attackPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vca_attackpickUp = false;
      vca_attack = value / readresdivider;
      updatevca_attack();
      vca_attackPrevValue = vca_attack; //PICK-UP
      break;

    case CCvcf_decay:
      if (pickUpActive && vcf_decaypickUp && ((vcf_decayPrevValue + TOLERANCE) <  (value) || (vcf_decayPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vcf_decaypickUp = false;
      vcf_decay = value / readresdivider;
      updatevcf_decay();
      vcf_decayPrevValue = vcf_decay; //PICK-UP
      break;

    case CCvca_release:
      if (pickUpActive && vca_releasepickUp && ((vca_releasePrevValue + TOLERANCE) <  (value) || (vca_releasePrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vca_releasepickUp = false;
      vca_release = value / readresdivider;
      updatevca_release();
      vca_releasePrevValue = vca_release; //PICK-UP
      break;

    case CCvcf_frequency:
      if (Detune == true)
      {
        voice5_detune = DETUNE[value / readresdivider];
        updatevcf_frequency();
      }
      else
      {
        if (pickUpActive && vcf_frequencypickUp && ((vcf_frequencyPrevValue + TOLERANCE) <  (value) || (vcf_frequencyPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        vcf_frequencypickUp = false;
        vcf_frequency =  value / readresdivider;
        updatevcf_frequency();
        vcf_frequencyPrevValue = vcf_frequency; //PICK-UP
      }
      break;

    case CCosc_2_detune:
      if (pickUpActive && osc_2_detunepickUp && ((osc_2_detunePrevValue + TOLERANCE) <  (value) || (osc_2_detunePrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      osc_2_detunepickUp = false;
      osc_2_detune =  value / readresdivider;
      updateosc_2_detune();
      osc_2_detunePrevValue = osc_2_detune; //PICK-UP
      break;

    case CClfo_frequency:
      if (pickUpActive && lfo_frequencypickUp && ((lfo_frequencyPrevValue + TOLERANCE) <  (value) || (lfo_frequencyPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      lfo_frequencypickUp = false;
      lfo_frequency =  value / readresdivider;
      updatelfo_frequency();
      lfo_frequencyPrevValue = lfo_frequency; //PICK-UP
      break;

    case CCportamento:
      if (pickUpActive && portamentopickUp && ((portamentoPrevValue + TOLERANCE) <  (value) || (portamentoPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      portamentopickUp = false;
      portamento =  value / readresdivider;
      updateportamento();
      portamentoPrevValue = portamento; //PICK-UP
      break;

    case CCvca_decay:
      if (pickUpActive && vca_decaypickUp && ((vca_decayPrevValue + TOLERANCE) <  (value) || (vca_decayPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vca_decaypickUp = false;
      vca_decay =  value / readresdivider;
      updatevca_decay();
      vca_decayPrevValue = vca_decay; //PICK-UP
      break;

    case CCvcf_sustain:
      if (pickUpActive && vcf_sustainpickUp && ((vcf_sustainPrevValue + TOLERANCE) <  (value) || (vcf_sustainPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vcf_sustainpickUp = false;
      vcf_sustain =  value / readresdivider;
      updatevcf_sustain();
      vcf_sustainPrevValue = vcf_sustain; //PICK-UP
      break;

    case CCvca_sustain:
      if (pickUpActive && vca_sustainpickUp && ((vca_sustainPrevValue + TOLERANCE) <  (value) || (vca_sustainPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vca_sustainpickUp = false;
      vca_sustain =  value / readresdivider;
      updatevca_sustain();
      vca_sustainPrevValue = vca_sustain; //PICK-UP
      break;

    case CCvcf_release:
      if (pickUpActive && vcf_releasepickUp && ((vcf_releasePrevValue + TOLERANCE) <  (value) || (vcf_releasePrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vcf_releasepickUp = false;
      vcf_release =  value / readresdivider;
      updatevcf_release();
      vcf_releasePrevValue = vcf_release; //PICK-UP
      break;

    case CCvcf_resonance:
      if (Detune == true)
      {
        voice6_detune =  DETUNE[value / readresdivider];
        updatevcf_resonance();
      }
      else
      {
        if (pickUpActive && vcf_resonancepickUp && ((vcf_resonancePrevValue + TOLERANCE) <  (value) || (vcf_resonancePrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        vcf_resonancepickUp = false;
        vcf_resonance =  value / readresdivider;
        updatevcf_resonance();
        vcf_resonancePrevValue = vcf_resonance; //PICK-UP
      }
      break;

    case CCvcf_env_amount:
      if (pickUpActive && vcf_env_amountpickUp && ((vcf_env_amountPrevValue + TOLERANCE) <  (value) || (vcf_env_amountPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      vcf_env_amountpickUp = false;
      vcf_env_amount =  value / readresdivider;
      updatevcf_env_amount();
      vcf_env_amountPrevValue = vcf_env_amount; //PICK-UP
      break;

    case CCpulsewidth:
      if (Detune == true)
      {
        voice2_detune =  DETUNE[value / readresdivider];
        updatepulsewidth();
      }
      else
      {
        if (pickUpActive && pulsewidthpickUp && ((pulsewidthPrevValue + TOLERANCE) <  (value) || (pulsewidthPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        pulsewidthpickUp = false;
        pulsewidth =  value / readresdivider;
        updatepulsewidth();
        pulsewidthPrevValue = pulsewidth; //PICK-UP
      }
      break;

    case CCmodulation:
      if (pickUpActive && modulationpickUp && ((modulationPrevValue + TOLERANCE) <  (value) || (modulationPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
      modulationpickUp = false;
      modulation =  value / readresdivider;
      updatemodulation();
      modulationPrevValue = modulation; //PICK-UP
      break;

    case CCoscpwm:
      if (Detune == true)
      {
        voice3_detune =  DETUNE[value / readresdivider];
        updateoscpwm();
      }
      else
      {
        if (pickUpActive && oscpwmpickUp && ((oscpwmPrevValue + TOLERANCE) <  (value) || (oscpwmPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        oscpwmpickUp = false;
        oscpwm =  value / readresdivider;
        updateoscpwm();
        oscpwmPrevValue = oscpwm; //PICK-UP
      }
      break;

    case CCosc_2_freq:
      if (Detune == true)
      {
        voice4_detune =  DETUNE[value / readresdivider];
        updateosc_2_freq();
      }
      else
      {
        if (pickUpActive && osc_2_freqpickUp && ((osc_2_freqPrevValue + TOLERANCE) <  (value) || (osc_2_freqPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        osc_2_freqpickUp = false;
        osc_2_freq =  value / readresdivider;
        updateosc_2_freq();
        osc_2_freqPrevValue = osc_2_freq; //PICK-UP
      }
      break;

    case CCosc_1_freq:
      if (Detune == true)
      {
        voice1_detune =  DETUNE[value / readresdivider];
        updateosc_1_freq();
      }
      else
      {
        if (pickUpActive && osc_1_freqpickUp && ((osc_1_freqPrevValue + TOLERANCE) <  (value) || (osc_1_freqPrevValue - TOLERANCE) >  (value))) return; //PICK-UP
        osc_1_freqpickUp = false;
        osc_1_freq =  value / readresdivider;
        updateosc_1_freq();
        osc_1_freqPrevValue = osc_1_freq; //PICK-UP
      }
      break;

    case CCunison:
      value > 0 ? unison = 127 : unison = 0;
      updateUnison();
      break;

    case CCosc_2_fm:
      value > 0 ? osc_2_fm = 127 : osc_2_fm = 0;
      updateosc_2_fm();
      break;

    case CCvcf_mod:
      value > 0 ? vcf_mod = 127 : vcf_mod = 0;
      updatevcf_mod();
      break;

    case CCosc_1_wave:
      value > 0 ? osc_1_wave = 127 : osc_1_wave = 0;
      updateosc_1_wave();
      break;

    case CCosc_2_wave:
      value > 0 ? osc_2_wave = 127 : osc_2_wave = 0;
      updateosc_2_wave();
      break;

    case CClfo_wave:
      value > 0 ? lfo_wave = 127 : lfo_wave = 0;
      updatelfo_wave();
      break;

    case CCosc_1_fm:
      value > 0 ? osc_1_fm = 127 : osc_1_fm = 0;
      updateosc_1_fm();
      break;

    case CCosc_1_pwm:
      value > 0 ? osc_1_pwm = 127 : osc_1_pwm = 0;
      updateosc_1_pwm();
      break;

    case CCosc_2_pwm:
      value > 0 ? osc_2_pwm = 127 : osc_2_pwm = 0;
      updateosc_2_pwm();
      break;

    case CCvcf_track:
      value > 0 ? vcf_track = 127 : vcf_track = 0;
      updatevcf_track();
      break;

    case CCosc_2_half:
      value > 0 ? osc_2_half = 127 : osc_2_half = 0;
      updateosc_2_half();
      break;

    case CCsync:
      value > 0 ? sync = 127 : sync = 0;
      updatesync();
      break;

    case CCcrossmod:
      value > 0 ? crossmod = 127 : crossmod = 0;
      updatecrossmod();
      break;
  }
}

void myProgramChange(byte channel, byte program)
{
  state = PATCH;
  patchNo = program + 1;
  recallPatch(patchNo);
  Serial.print("MIDI Pgm Change:");
  Serial.println(patchNo);
  state = PARAMETER;
}

void recallPatch(int patchNo)
{
  allNotesOff();
  File patchFile = SD.open(String(patchNo).c_str());
  if (!patchFile)
  {
    Serial.println("File not found");
  }
  else
  {
    String data[NO_OF_PARAMS]; //Array of data read in
    recallPatchData(patchFile, data);
    setCurrentPatchData(data);
    patchFile.close();
  }
}

void setCurrentPatchData(String data[])
{
  patchName = data[0];
  vcf_attack = data[1].toInt();
  vcf_attackPrevValue = vcf_attack * 2;
  vcf_attackpickUp = true;
  vca_attack = data[2].toInt();
  vca_attackPrevValue = vca_attack * 2;
  vca_attackpickUp = true;
  vcf_decay = data[3].toInt();
  vcf_decayPrevValue = vcf_decay * 2;
  vcf_decaypickUp = true;
  vca_release = data[4].toInt();
  vca_releasePrevValue = vca_release * 2;
  vca_releasepickUp = true;
  vcf_frequency = data[5].toInt();
  vcf_frequencyPrevValue = vcf_frequency * 2;
  vcf_frequencypickUp = true;
  osc_2_detune = data[6].toInt();
  osc_2_detunePrevValue = osc_2_detune * 2;
  osc_2_detunepickUp = true;
  lfo_frequency = data[7].toInt();
  lfo_frequencyPrevValue = lfo_frequency * 2;
  lfo_frequencypickUp = true;
  portamento = data[8].toInt();
  portamentoPrevValue = portamento * 2;
  portamentopickUp = true;
  vca_decay = data[9].toInt();
  vca_decayPrevValue = vca_decay * 2;
  vca_decaypickUp = true;
  vcf_sustain = data[10].toInt();
  vcf_sustainPrevValue = vcf_sustain * 2;
  vcf_sustainpickUp = true;
  vca_sustain = data[11].toInt();
  vca_sustainPrevValue = vca_sustain * 2;
  vca_sustainpickUp = true;
  vcf_release = data[12].toInt();
  vcf_releasePrevValue = vcf_release * 2;
  vcf_releasepickUp = true;
  vcf_resonance = data[13].toInt();
  vcf_resonancePrevValue = vcf_resonance * 2;
  vcf_resonancepickUp = true;
  vcf_env_amount = data[14].toInt();
  vcf_env_amountPrevValue = vcf_env_amount * 2;
  vcf_env_amountpickUp = true;
  pulsewidth = data[15].toInt();
  pulsewidthPrevValue = pulsewidth * 2;
  pulsewidthpickUp = true;
  modulation = data[16].toInt();
  modulationPrevValue = modulation * 2;
  modulationpickUp = true;
  oscpwm = data[17].toInt();
  oscpwmPrevValue = oscpwm * 2;
  oscpwmpickUp = true;
  osc_2_freq = data[18].toInt();
  osc_2_freqPrevValue = osc_2_freq * 2;
  osc_2_freqpickUp = true;
  osc_1_freq = data[19].toInt();
  osc_1_freqPrevValue = osc_1_freq * 2;
  osc_1_freqpickUp = true;

  osc_2_fm = data[20].toInt();
  vcf_mod = data[21].toInt();
  osc_1_wave = data[22].toInt();
  osc_2_wave = data[23].toInt();
  lfo_wave = data[24].toInt();
  osc_1_fm = data[25].toInt();
  osc_1_pwm = data[26].toInt();
  osc_2_pwm = data[27].toInt();
  vcf_track = data[28].toInt();
  osc_2_half = data[29].toInt();
  sync = data[30].toInt();
  crossmod = data[31].toInt();
  unison = data[32].toInt();

  voice1_detune = data[33].toInt();
  voice2_detune = data[34].toInt();
  voice3_detune = data[35].toInt();
  voice4_detune = data[36].toInt();
  voice5_detune = data[37].toInt();
  voice6_detune = data[38].toInt();

  midiCCOut(CCvcf_attack, vcf_attack, 1);
  delay(1);
  midiCCOut(CCvca_attack, vca_attack, 1);
  delay(1);
  midiCCOut(CCvcf_decay, vcf_decay, 1);
  delay(1);
  midiCCOut(CCvca_release, vca_release, 1);
  delay(1);
  midiCCOut(CCvcf_frequency, vcf_frequency, 1);
  delay(1);
  midiCCOut(CCosc_2_detune, osc_2_detune, 1);
  delay(1);
  midiCCOut(CClfo_frequency, lfo_frequency, 1);
  delay(1);
  midiCCOut(CCportamento, portamento, 1);
  delay(1);
  midiCCOut(CCvca_decay, vca_decay, 1);
  delay(1);
  midiCCOut(CCvcf_sustain, vcf_sustain, 1);
  delay(1);
  midiCCOut(CCvca_sustain, vca_sustain, 1);
  delay(1);
  midiCCOut(CCvcf_release, vcf_release, 1);
  delay(1);
  midiCCOut(CCvcf_resonance, LINEAR_INVERSE[vcf_resonance], 1);
  delay(1);
  midiCCOut(CCvcf_env_amount, vcf_env_amount, 1);
  delay(1);
  midiCCOut(CCpulsewidth, pulsewidth, 1);
  delay(1);
  midiCCOut(CCmodulation, modulation, 1);
  delay(1);
  midiCCOut(CCoscpwm, oscpwm, 1);
  delay(1);
  midiCCOut(CCosc_2_freq, osc_2_freq, 1);
  delay(1);
  midiCCOut(CCosc_1_freq, osc_1_freq, 1);
  delay(1);

  patchRecall = true;
  updateosc_2_fm();
  updatevcf_mod();
  updateosc_1_wave();
  updateosc_2_wave();
  updatelfo_wave();
  updateosc_1_fm();
  updateosc_1_pwm();
  updateosc_2_pwm();
  updatevcf_track();
  updateosc_2_half();
  updatesync();
  updatecrossmod();
  updateUnison();
  patchRecall = false;

  if (unison == 127)
  {
    midiCCOut(108, voice1_detune, 1);
    midiCCOut(108, voice2_detune, 2);
    midiCCOut(108, voice3_detune, 3);
    midiCCOut(108, voice4_detune, 4);
    midiCCOut(108, voice5_detune, 5);
    midiCCOut(108, voice6_detune, 6);
  }

  //Patchname
  updatePatchname();

  Serial.print("Set Patch: ");
  Serial.println(patchName);
}

String getCurrentPatchData()
{
  return patchName + "," + String(vcf_attack) + "," + String(vca_attack) + "," + String(vcf_decay) + "," + String(vca_release) + "," + String(vcf_frequency) + "," + String(osc_2_detune) + "," + String(lfo_frequency) + "," + String(portamento) + "," +
         String(vca_decay) + "," + String(vcf_sustain) + "," + String(vca_sustain) + "," + String(vcf_release) + "," + String(vcf_resonance) + "," + String(vcf_env_amount) + "," + String(pulsewidth) + "," + String(modulation) + "," + String(oscpwm) + "," +
         String(osc_2_freq) + "," + String(osc_1_freq) + "," +
         String(osc_2_fm) + "," + String(vcf_mod) + "," + String(osc_1_wave) + "," + String(osc_2_wave) + "," + String(lfo_wave) + "," + String(osc_1_fm) + "," + String(osc_1_pwm) + "," + String(osc_2_pwm) + "," + String(vcf_track) + "," +
         String(osc_2_half) + "," + String(sync) + "," + String(crossmod) + "," + String(unison) + "," +
         String(voice1_detune) + "," + String(voice2_detune) + "," + String(voice3_detune) + "," + String(voice4_detune) + "," + String(voice5_detune) + "," + String(voice6_detune);
}

void checkMux()
{

  mux1Read = adc->adc1->analogRead(MUX1_S);
  mux2Read = adc->adc1->analogRead(MUX2_S);

  if (mux1Read > (mux1ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux1Read < (mux1ValuesPrev[muxInput] - QUANTISE_FACTOR))
  {
    mux1ValuesPrev[muxInput] = mux1Read;

    switch (muxInput)
    {
      case MUX1_OSC1_1_FREQ:
        myControlChange(midiChannel, CCosc_1_freq, mux1Read);
        break;
      case MUX1_PULSEWIDTH:
        myControlChange(midiChannel, CCpulsewidth, mux1Read);
        break;
      case MUX1_PWM:
        myControlChange(midiChannel, CCoscpwm, mux1Read);
        break;
      case MUX1_OSC1_2_FREQ:
        myControlChange(midiChannel, CCosc_2_freq, mux1Read);
        break;
      case MUX1_VCF_FREQ:
        myControlChange(midiChannel, CCvcf_frequency, mux1Read);
        break;
      case MUX1_RESONANCE:
        myControlChange(midiChannel, CCvcf_resonance, mux1Read);
        break;
      case MUX1_AMOUNT:
        myControlChange(midiChannel, CCvcf_env_amount, mux1Read);
        break;
      case MUX1_LFO_RATE:
        myControlChange(midiChannel, CClfo_frequency, mux1Read);
        break;
      case MUX1_MODULATION:
        myControlChange(midiChannel, CCmodulation, mux1Read);
        break;
      case MUX1_ATTACK:
        myControlChange(midiChannel, CCvcf_attack, mux1Read);
        break;
      case MUX1_DECAY:
        myControlChange(midiChannel, CCvcf_decay, mux1Read);
        break;
      case MUX1_SUSTAIN:
        myControlChange(midiChannel, CCvcf_sustain, mux1Read);
        break;
      case MUX1_RELEASE:
        myControlChange(midiChannel, CCvcf_release, mux1Read);
        break;
    }
  }

  if (mux2Read > (mux2ValuesPrev[muxInput] + QUANTISE_FACTOR) || mux2Read < (mux2ValuesPrev[muxInput] - QUANTISE_FACTOR))
  {
    mux2ValuesPrev[muxInput] = mux2Read;

    switch (muxInput)
    {
      case MUX2_ATTACK:
        myControlChange(midiChannel, CCvca_attack, mux2Read);
        break;
      case MUX2_DECAY:
        myControlChange(midiChannel, CCvca_decay, mux2Read);
        break;
      case MUX2_SUSTAIN:
        myControlChange(midiChannel, CCvca_sustain, mux2Read);
        break;
      case MUX2_RELEASE:
        myControlChange(midiChannel, CCvca_release, mux2Read);
        break;
      case MUX2_OSC_2_DETUNE:
        myControlChange(midiChannel, CCosc_2_detune, mux2Read);
        break;
      case MUX2_PORTAMENTO:
        myControlChange(midiChannel, CCportamento, mux2Read);
        break;
    }
  }

  muxInput++;
  if (muxInput >= MUXCHANNELS)
    muxInput = 0;

  digitalWriteFast(MUX_0, muxInput & B0001);
  digitalWriteFast(MUX_1, muxInput & B0010);
  digitalWriteFast(MUX_2, muxInput & B0100);
  digitalWriteFast(MUX_3, muxInput & B1000);

}

void midiCCOut(byte cc, byte value, byte ccChannel)
{
  MIDI.sendControlChange(cc, value, ccChannel); //MIDI DIN is set to Out
  usbMIDI.sendControlChange(cc, value, ccChannel); //MIDI DIN is set to Out
}

void midiProgOut(byte chg, byte channel)
{
  if (Program == true)
  {
    if (chg < 113 )
    {
      MIDI.sendProgramChange(chg, channel); //MIDI DIN is set to Out
      usbMIDI.sendProgramChange(chg, channel); //MIDI DIN is set to Out
    }
  }
}

void onButtonPress(uint16_t btnIndex, uint8_t btnType) {

  // to check if a specific button was pressed

  if (btnIndex == OSC_1_WAVE && btnType == ROX_PRESSED) {
    osc_1_wave = !osc_1_wave;
    myControlChange(midiChannel, CCosc_1_wave, osc_1_wave);
  }

  if (btnIndex == CROSSMOD && btnType == ROX_PRESSED) {
    crossmod = !crossmod;
    myControlChange(midiChannel, CCcrossmod, crossmod);
  }

  if (btnIndex == OSC_1_PWM && btnType == ROX_PRESSED) {
    osc_1_pwm = !osc_1_pwm;
    myControlChange(midiChannel, CCosc_1_pwm, osc_1_pwm);
  }

  if (btnIndex == OSC_2_WAVE && btnType == ROX_PRESSED) {
    osc_2_wave = !osc_2_wave;
    myControlChange(midiChannel, CCosc_2_wave, osc_2_wave);
  }

  if (btnIndex == OSC_2_HALF && btnType == ROX_PRESSED) {
    osc_2_half = !osc_2_half;
    myControlChange(midiChannel, CCosc_2_half, osc_2_half);
  }

  if (btnIndex == SYNC && btnType == ROX_PRESSED) {
    sync = !sync;
    myControlChange(midiChannel, CCsync, sync);
  }

  if (btnIndex == OSC_2_PWM && btnType == ROX_PRESSED) {
    osc_2_pwm = !osc_2_pwm;
    myControlChange(midiChannel, CCosc_2_pwm, osc_2_pwm);
  }

  if (btnIndex == UNISON && btnType == ROX_PRESSED) {
    unison = !unison;
    myControlChange(midiChannel, CCunison, unison);
  }

  if (btnIndex == VCF_MOD && btnType == ROX_PRESSED) {
    vcf_mod = !vcf_mod;
    myControlChange(midiChannel, CCvcf_mod, vcf_mod);
  }

  if (btnIndex == KEYTRACK && btnType == ROX_PRESSED) {
    vcf_track = !vcf_track;
    myControlChange(midiChannel, CCvcf_track, vcf_track);
  }

  if (btnIndex == LFO_WAVE && btnType == ROX_PRESSED) {
    lfo_wave = !lfo_wave;
    myControlChange(midiChannel, CClfo_wave, lfo_wave);
  }

  if (btnIndex == OSC_1_MOD && btnType == ROX_PRESSED) {
    osc_1_fm = !osc_1_fm;
    myControlChange(midiChannel, CCosc_1_fm, osc_1_fm);
  }

  if (btnIndex == OSC_2_MOD && btnType == ROX_PRESSED) {
    osc_2_fm = !osc_2_fm;
    myControlChange(midiChannel, CCosc_2_fm, osc_2_fm);
  }
}

void checkSwitches()
{
  if (Voices != oldVoices)
  {
    midiCCOut(120, Voices, 16);
    oldVoices = Voices;
  }

  saveButton.update();
  if (saveButton.read() == LOW && saveButton.duration() > HOLD_DURATION)
  {
    switch (state)
    {
      case PARAMETER:
      case PATCH:
        state = DELETE;
        saveButton.write(HIGH); //Come out of this state
        del = true;             //Hack
        break;
    }
  }
  else if (saveButton.risingEdge())
  {
    if (!del)
    {
      switch (state)
      {
        case PARAMETER:
          if (patches.size() < PATCHES_LIMIT)
          {
            resetPatchesOrdering(); //Reset order of patches from first patch
            patches.push({patches.size() + 1, INITPATCHNAME});
            state = SAVE;
          }
          break;
        case SAVE:
          //Save as new patch with INITIALPATCH name or overwrite existing keeping name - bypassing patch renaming
          patchName = patches.last().patchName;
          state = PATCH;
          savePatch(String(patches.last().patchNo).c_str(), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patches.last().patchName);
          patchNo = patches.last().patchNo;
          loadPatches(); //Get rid of pushed patch if it wasn't saved
          setPatchesOrdering(patchNo);
          renamedPatch = "";
          state = PARAMETER;
          break;
        case PATCHNAMING:
          if (renamedPatch.length() > 0) patchName = renamedPatch;//Prevent empty strings
          state = PATCH;
          savePatch(String(patches.last().patchNo).c_str(), getCurrentPatchData());
          showPatchPage(patches.last().patchNo, patchName);
          patchNo = patches.last().patchNo;
          loadPatches(); //Get rid of pushed patch if it wasn't saved
          setPatchesOrdering(patchNo);
          renamedPatch = "";
          state = PARAMETER;
          break;
      }
    }
    else
    {
      del = false;
    }
  }

  settingsButton.update();
  if (settingsButton.read() == LOW && settingsButton.duration() > HOLD_DURATION)
  {
    //If recall held, set current patch to match current hardware state
    //Reinitialise all hardware values to force them to be re-read if different
    state = REINITIALISE;
    reinitialiseToPanel();
    settingsButton.write(HIGH); //Come out of this state
    reini = true;           //Hack
  }
  else if (settingsButton.risingEdge())
  { //cannot be fallingEdge because holding button won't work
    if (!reini)
    {
      switch (state)
      {
        case PARAMETER:
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
        case SETTINGS:
          settingsOptions.push(settingsOptions.shift());
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        case SETTINGSVALUE:
          //Same as pushing Recall - store current settings item and go back to options
          settingsHandler(settingsOptions.first().value[settingsValueIndex], settingsOptions.first().handler);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      reini = false;
    }
  }

  backButton.update();
  if (backButton.read() == LOW && backButton.duration() > HOLD_DURATION)
  {
    //If Back button held, Panic - all notes off
    allNotesOff();
    backButton.write(HIGH); //Come out of this state
    panic = true;           //Hack
  }
  else if (backButton.risingEdge())
  { //cannot be fallingEdge because holding button won't work
    if (!panic)
    {
      switch (state)
      {
        case RECALL:
          setPatchesOrdering(patchNo);
          state = PARAMETER;
          break;
        case SAVE:
          renamedPatch = "";
          state = PARAMETER;
          loadPatches();//Remove patch that was to be saved
          setPatchesOrdering(patchNo);
          break;
        case PATCHNAMING:
          charIndex = 0;
          renamedPatch = "";
          state = SAVE;
          break;
        case DELETE:
          setPatchesOrdering(patchNo);
          state = PARAMETER;
          break;
        case SETTINGS:
          state = PARAMETER;
          break;
        case SETTINGSVALUE:
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      panic = false;
    }
  }

  //Encoder switch
  recallButton.update();
  if (recallButton.read() == LOW && recallButton.duration() > HOLD_DURATION)
  {
    //If Recall button held, return to current patch setting
    //which clears any changes made
    state = PATCH;
    //Recall the current patch
    patchNo = patches.first().patchNo;
    recallPatch(patchNo);
    state = PARAMETER;
    recallButton.write(HIGH); //Come out of this state
    recall = true;            //Hack
  }
  else if (recallButton.risingEdge())
  {
    if (!recall)
    {
      switch (state)
      {
        case PARAMETER:
          state = RECALL;//show patch list
          break;
        case RECALL:
          state = PATCH;
          //Recall the current patch
          patchNo = patches.first().patchNo;
          recallPatch(patchNo);
          state = PARAMETER;
          break;
        case SAVE:
          showRenamingPage(patches.last().patchName);
          patchName  = patches.last().patchName;
          state = PATCHNAMING;
          break;
        case PATCHNAMING:
          if (renamedPatch.length() < 13)
          {
            renamedPatch.concat(String(currentCharacter));
            charIndex = 0;
            currentCharacter = CHARACTERS[charIndex];
            showRenamingPage(renamedPatch);
          }
          break;
        case DELETE:
          //Don't delete final patch
          if (patches.size() > 1)
          {
            state = DELETEMSG;
            patchNo = patches.first().patchNo;//PatchNo to delete from SD card
            patches.shift();//Remove patch from circular buffer
            deletePatch(String(patchNo).c_str());//Delete from SD card
            loadPatches();//Repopulate circular buffer to start from lowest Patch No
            renumberPatchesOnSD();
            loadPatches();//Repopulate circular buffer again after delete
            patchNo = patches.first().patchNo;//Go back to 1
            recallPatch(patchNo);//Load first patch
          }
          state = PARAMETER;
          break;
        case SETTINGS:
          //Choose this option and allow value choice
          settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGSVALUE);
          state = SETTINGSVALUE;
          break;
        case SETTINGSVALUE:
          //Store current settings item and go back to options
          settingsHandler(settingsOptions.first().value[settingsValueIndex], settingsOptions.first().handler);
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
          state = SETTINGS;
          break;
      }
    }
    else
    {
      recall = false;
    }
  }
}

void reinitialiseToPanel()
{
  //This sets the current patch to be the same as the current hardware panel state - all the pots
  //The four button controls stay the same state
  //This reinialises the previous hardware values to force a re-read
  muxInput = 0;
  for (int i = 0; i < MUXCHANNELS; i++)
  {
    mux1ValuesPrev[i] = RE_READ;
    mux2ValuesPrev[i] = RE_READ;

  }
  patchName = INITPATCHNAME;
  showPatchPage("Initial", "Panel Settings");
}

void checkEncoder()
{
  //Encoder works with relative inc and dec values
  //Detent encoder goes up in 4 steps, hence +/-3

  long encRead = encoder.read();
  if ((encCW && encRead > encPrevious + 3) || (!encCW && encRead < encPrevious - 3) )
  {
    switch (state)
    {
      case PARAMETER:
        state = PATCH;
        patches.push(patches.shift());
        patchNo = patches.first().patchNo;
        recallPatch(patchNo);
        midiProgOut(patchNo, 1);
        state = PARAMETER;
        break;
      case RECALL:
        patches.push(patches.shift());
        break;
      case SAVE:
        patches.push(patches.shift());
        break;
      case PATCHNAMING:
        if (charIndex == TOTALCHARS) charIndex = 0;//Wrap around
        currentCharacter = CHARACTERS[charIndex++];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.push(patches.shift());
        break;
      case SETTINGS:
        settingsOptions.push(settingsOptions.shift());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex] , SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsOptions.first().value[settingsValueIndex + 1] != '\0')
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[++settingsValueIndex], SETTINGSVALUE);
        break;
    }
    encPrevious = encRead;
  }
  else if ((encCW && encRead < encPrevious - 3) || (!encCW && encRead > encPrevious + 3))
  {
    switch (state)
    {
      case PARAMETER:
        state = PATCH;
        patches.unshift(patches.pop());
        patchNo = patches.first().patchNo;
        recallPatch(patchNo);
        midiProgOut(patchNo, 1);
        state = PARAMETER;
        break;
      case RECALL:
        patches.unshift(patches.pop());
        break;
      case SAVE:
        //if (patchNo < 57 ) patchNo = 57;
        patches.unshift(patches.pop());
        break;
      case PATCHNAMING:
        if (charIndex == -1)
          charIndex = TOTALCHARS - 1;
        currentCharacter = CHARACTERS[charIndex--];
        showRenamingPage(renamedPatch + currentCharacter);
        break;
      case DELETE:
        patches.unshift(patches.pop());
        break;
      case SETTINGS:
        settingsOptions.unshift(settingsOptions.pop());
        settingsValueIndex = getCurrentIndex(settingsOptions.first().currentIndex);
        showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[settingsValueIndex], SETTINGS);
        break;
      case SETTINGSVALUE:
        if (settingsValueIndex > 0)
          showSettingsPage(settingsOptions.first().option, settingsOptions.first().value[--settingsValueIndex], SETTINGSVALUE);
        break;
    }
    encPrevious = encRead;
  }
}

void loop()
{
  checkMux();
  octoswitch.update();
  checkSwitches();
  checkEncoder();
  sr.update();
  MIDI.read(midiChannel);
  usbMIDI.read(midiChannel);
}
