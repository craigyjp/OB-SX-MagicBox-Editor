// This optional setting causes Encoder to use more optimized code,
// It must be defined before Encoder.h is included.
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Bounce.h>
#include <ADC.h>
#include <ADC_util.h>

ADC *adc = new ADC();

//Teensy 3.6 - Mux Pins
#define MUX_0 28
#define MUX_1 32
#define MUX_2 30
#define MUX_3 31

#define MUX1_S A2 // ADC1
#define MUX2_S A0 // ADC1

//Mux 1 Connections
#define MUX1_OSC1_1_FREQ 0
#define MUX1_PULSEWIDTH 1
#define MUX1_PWM 2
#define MUX1_OSC1_2_FREQ 3
#define MUX1_VCF_FREQ 4
#define MUX1_RESONANCE 5
#define MUX1_AMOUNT 6
#define MUX1_LFO_RATE 7
#define MUX1_MODULATION 8
#define MUX1_ATTACK 9
#define MUX1_DECAY 10
#define MUX1_SUSTAIN 11
#define MUX1_RELEASE 12
#define MUX1_spare13 13
#define MUX1_spare14 14
#define MUX1_spare15 15

//Mux 2 Connections
#define MUX2_ATTACK 0
#define MUX2_DECAY 1
#define MUX2_SUSTAIN 2
#define MUX2_RELEASE 3
#define MUX2_OSC_2_DETUNE 4
#define MUX2_PORTAMENTO 5
#define MUX2_spare6 6
#define MUX2_spare7 7
#define MUX2_spare8 8
#define MUX2_spare9 9
#define MUX2_spare10 10
#define MUX2_spare11 11
#define MUX2_spare12 12
#define MUX2_spare13 13
#define MUX2_spare14 14
#define MUX2_spare15 15

//Teensy 3.6 Pins

#define RECALL_SW 17
#define SAVE_SW 24
#define SETTINGS_SW 12
#define BACK_SW 10

#define ENCODER_PINA 5
#define ENCODER_PINB 4

// Switches

#define OSC_1_WAVE 0
#define CROSSMOD 1
#define OSC_1_PWM 2
#define OSC_2_WAVE 3
#define OSC_2_HALF 4
#define SYNC 5
#define OSC_2_PWM 6
#define UNISON 7

#define VCF_MOD 8
#define KEYTRACK 9
#define LFO_WAVE 10
#define OSC_1_MOD 11
#define OSC_2_MOD 12

#define DEBOUNCE 30

// LEDs

#define OSC_1_WAVE_LED 0
#define CROSSMOD_LED 1
#define OSC_1_PWM_LED 2
#define OSC_2_WAVE_LED 3
#define OSC_2_HALF_LED 4
#define SYNC_LED 5
#define OSC_2_PWM_LED 6
#define UNISON_LED 7

#define VCF_MOD_LED 8
#define KEYTRACK_LED 9
#define LFO_WAVE_LED 10
#define OSC_1_MOD_LED 11
#define OSC_2_MOD_LED 12

#define MUXCHANNELS 16
#define QUANTISE_FACTOR 1

static byte muxInput = 0;
static int mux1ValuesPrev[MUXCHANNELS] = {};
static int mux2ValuesPrev[MUXCHANNELS] = {};


static int mux1Read = 0;
static int mux2Read = 0;


static long encPrevious = 0;

//These are pushbuttons and require debouncing

Bounce recallButton = Bounce(RECALL_SW, DEBOUNCE); //On encoder
boolean recall = true; //Hack for recall button
Bounce saveButton = Bounce(SAVE_SW, DEBOUNCE);
boolean del = true; //Hack for save button
Bounce settingsButton = Bounce(SETTINGS_SW, DEBOUNCE);
boolean reini = true; //Hack for settings button
Bounce backButton = Bounce(BACK_SW, DEBOUNCE);
boolean panic = true; //Hack for back button
Encoder encoder(ENCODER_PINB, ENCODER_PINA);//This often needs the pins swapping depending on the encoder

void setupHardware()
{
     //Volume Pot is on ADC0
  adc->adc0->setAveraging(32); // set number of averages 0, 4, 8, 16 or 32.
  adc->adc0->setResolution(8); // set bits of resolution  8, 10, 12 or 16 bits.
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed

  //MUXs on ADC1
  adc->adc1->setAveraging(32); // set number of averages 0, 4, 8, 16 or 32.
  adc->adc1->setResolution(8); // set bits of resolution  8, 10, 12 or 16 bits.
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed


  //Mux address pins

  pinMode(MUX_0, OUTPUT);
  pinMode(MUX_1, OUTPUT);
  pinMode(MUX_2, OUTPUT);
  pinMode(MUX_3, OUTPUT);

  digitalWrite(MUX_0, LOW);
  digitalWrite(MUX_1, LOW);
  digitalWrite(MUX_2, LOW);
  digitalWrite(MUX_3, LOW);

  analogReadResolution(8);

  //Switches
  pinMode(RECALL_SW, INPUT_PULLUP); //On encoder
  pinMode(SAVE_SW, INPUT_PULLUP);
  pinMode(SETTINGS_SW, INPUT_PULLUP);
  pinMode(BACK_SW, INPUT_PULLUP);

}
