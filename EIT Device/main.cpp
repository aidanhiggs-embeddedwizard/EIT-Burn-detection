#include <Arduino.h>
#include "ADG732_16Channel.h"
#include <AD9833.h>
#include <Adafruit_NeoPixel.h>
#include <ADC.h>
#include <ADC_util.h>
#include <AnalogBufferDMA.h>
#include <math.h>

#define RGB_ring_PIN     22
#define BRIGHTNESS 50 // Set BRIGHTNESS -> 0 (min) to 255 (max)
//IQ Demodulation Configuration
#define START_BUTTON 5
#define ANALOG_PIN       15  // A15 pin for ADC input
#define NUM_CHANNELS     8   // Based on the 16-channel multiplexer chain
//for adjacent pattern
#define Num_measurements       NUM_CHANNELS*(NUM_CHANNELS - 3)   // N(N-3) calculation 

//for pseudo-adjacent pattern
//#define Num_measurements       NUM_CHANNELS*(NUM_CHANNELS - 4)


const int F_STIM = 25000;// 25kHz 50000; //50kHz stim frequency
const int F_SAMPLE = 500000; // 500kHz sampling rate
const int SamplesToCollect = 1000; // Buffer size for IQ stability
const int Sample_per_cycle = F_SAMPLE/F_STIM;
const int garbage_samples = 20;
const int Num_Valid_Samples = SamplesToCollect - garbage_samples;
//set up a lookup table for the IQ calculations
float sine_lut[Sample_per_cycle];
float cosine_lut[Sample_per_cycle];

//ADC and DMA setup

DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_buff1[SamplesToCollect];
DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_buff2[SamplesToCollect];

ADC *adc = new ADC();
AnalogBufferDMA abdma(dma_buff1, SamplesToCollect, dma_buff2, SamplesToCollect);

//#define RED_LED 2
//wave gen setup
AD9833 Wavegen(10,11,13); //CS, dat, clk

//Multiplexer setup
ADG732 InjectNeg(28,29,30,31,32); //ADG732(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t WR)
ADG732 InjectPos(9,24,25,26,27);

ADG732 VmeasureNeg(14,41,40,39,38);
ADG732 VmeasurePos(33,34,35,36,37);

//define posision of channels 
uint8_t Chan_pos_InjectPos = 0; //inital position = 0
uint8_t Chan_pos_InjectNeg = 0; //inital position = 1
uint8_t Chan_pos_VmeasPos = 0;  //inital position = 2
uint8_t Chan_pos_VmeasNeg = 0;  //inital position = 3


const int RED_LED = 2;
const int GREEN_LED = 3;
const int BLUE_LED = 4;


Adafruit_NeoPixel strip(NUM_CHANNELS, RGB_ring_PIN, NEO_GRB + NEO_KHZ800);

//setup functions

void SwitchMuxPattern(int step);
void waitForButtonPress();
//State machine
enum STATE {IDLE, MEASURING, inHOMOGENEOUS, measurementDONE};
STATE currentState =  IDLE;
bool isInhomogeneous_DONE = false;


float homog_data[Num_measurements];
float inhomog_data[Num_measurements];
int currentMuxStep = 0; //keep track of mux pos

//                    ---------------------------------------------------
//                    -------------------SETUP FUNCTION------------------
//                    ---------------------------------------------------
void setup(){

  Serial.begin(115200);
  SPI.begin();
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWriteFast(RED_LED, LOW);
  digitalWriteFast(GREEN_LED, LOW);
  digitalWriteFast(BLUE_LED, LOW);
  pinMode(START_BUTTON, INPUT_PULLUP);
  //start button:
  pinMode(START_BUTTON, INPUT_PULLUP);
  delay(1000);  // Give Serial time to initialize
  while(!Serial){};

  Serial.println("############################");
  Serial.println("SALINE TANK TESTS - Starting");
  Serial.println("############################");
  Serial.println("TEENSY ADC SETUP");
  Serial.printf("Fstim: %d\t|| Fsample: %d\t|| NUM_CHANNELS: %d|| Num_measurements: %d \t\n",F_STIM,F_SAMPLE,NUM_CHANNELS, Num_measurements); 

  //precompute Sine/Cosine LUT
  for(int i = 0; i<Sample_per_cycle; i++){
    sine_lut[i] = sin(2.0*PI*((float)i/Sample_per_cycle));
    cosine_lut[i] = cos((2.0*PI*((float)i/Sample_per_cycle)));
  }

  //Setup adc
  pinMode(ANALOG_PIN, INPUT);
  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);

  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);
  //adc->adc0->startSingleRead(ANALOG_PIN);
  abdma.init(adc, ADC_0);
  Serial.println("WAVE GEN SETUP");

  //Setup wavegen
  Wavegen.begin();
  Wavegen.setWave(AD9833_OFF); //Switch off wavegen


  Serial.println("RGB RING SETUP");
  strip.begin();
  strip.clear(); 
  strip.setBrightness(BRIGHTNESS);
  strip.show(); 

  Serial.println("REMEMBER: INJECT == RED; VMEAS == BLUE");
  Serial.println("STARTING NOW....");
  Serial.println("Calculating values for OMEGA");
  Serial.println("Waiting for Start button press");

}


void loop()
{


  switch (currentState)
  {
  case IDLE:
    /*
    after setup; if button pressed, set currentMuxStep = 0 && MuxSwitchPattern = 0, set isInhomogeneous_DONE to false
    Start dma timer
    set currentstate to measuring state 
    */
    //Serial.println("IDLE");
    //if(digitalRead(START_BUTTON) == LOW){

      digitalWriteFast(RED_LED, LOW);
      digitalWriteFast(GREEN_LED, LOW);
      digitalWriteFast(BLUE_LED, HIGH);
      Serial.println("REMEMBER TO CONNECT JUMPERS");
      waitForButtonPress(); // <--- BLOCKING WAIT
      Wavegen.begin();
      Wavegen.setWave(AD9833_SINE);
      Wavegen.setFrequency(F_STIM, 0);
      Wavegen.setFrequencyChannel(0);
      Wavegen.setPhase(0, 0);
      //delay(5000);
      isInhomogeneous_DONE = false;
      currentMuxStep = 0;
      //SwitchMuxPattern(0);
      //adc->adc0->analogRead(ANALOG_PIN);
      //delay(50); 
      //adc->adc0->startTimer(F_SAMPLE);

      currentState = MEASURING;
    //}
  break;
  case MEASURING:
    /* if DMA interrupted, read buffer, clear cache
        convert adc value to voltage, do dma demodulation, calculate magnitude
          if isInhomogeneous_DONE == true; store magnitude in inhomogenous[] else stor in homogenous[].
        increment currentMuxStep++
          if currentMuxStep < Num_measurements, set SwitchMuxPattern(currentMuxStep)
          else stop dma timer, set current state to inHOMOGENEOUS if isInhomogeneous_DONE == false, else state is measurementDONE
        clear interrupt flag (IMPORTANT!!!!!)

    */
    if(currentMuxStep < Num_measurements){
      SwitchMuxPattern(currentMuxStep);
      //delayMicroseconds(200);
      delay(5);
      abdma.clearInterrupt();
      adc->adc0->startSingleRead(ANALOG_PIN);  //prime adc
      //single shot DMA
      adc->adc0->startTimer(F_SAMPLE);
      //Serial.println("STARTED TIMER");
      while(!abdma.interrupted()){
        yield();
      }
      //Serial.println("STOPPED TIMER");
      adc->adc0->stopTimer();
      
      volatile uint16_t *pbuffer = abdma.bufferLastISRFilled();
      arm_dcache_delete((void *)pbuffer, sizeof(dma_buff1)); 

      float dc_sum = 0;
      for(int k = 0; k < SamplesToCollect; k++) {
          dc_sum += pbuffer[k];
      }
      float DC_offset = dc_sum / (float)SamplesToCollect;

      float Isum = 0, Qsum = 0;
      float Iavg = 0, Qavg = 0;
      //ADC to voltage
      for(int k = garbage_samples; k<SamplesToCollect; k++){
          float adc_voltage = (pbuffer[k] - DC_offset) * (0.000805664); // 3.3V/4096 for 12 bit resolution
          //removing dc offset
          //float adc_voltage = (pbuffer[k] ) * (0.000805664); // 3.3V/4096 for 12 bit resolution
          //IQ demod
          Isum += adc_voltage * sine_lut[k%Sample_per_cycle];
          Qsum += adc_voltage * cosine_lut[k%Sample_per_cycle];
      }    
      Iavg = Isum/ (SamplesToCollect - garbage_samples);
      Qavg = Qsum/ (SamplesToCollect - garbage_samples);
      //mag calculations
      float MAGNITUDE = 2.0f * sqrtf(Iavg*Iavg + Qavg*Qavg);

      /*int num_meas_per_inj = NUM_CHANNELS - 4; 
      int j = currentMuxStep % num_meas_per_inj; 

      // The first 6 measurements (j = 0 to 5) are "downhill" -> Positive
      // The last 6 measurements (j = 6 to 11) are "uphill" -> Negative
      if (j >= (num_meas_per_inj / 2)) {
          MAGNITUDE = -MAGNITUDE;
      }*/
      
      

      if(isInhomogeneous_DONE == true){
          //store magnitude in inhomoData
          inhomog_data[currentMuxStep] = MAGNITUDE;
      }
      else{
        //stor magnitude in homodata
        homog_data[currentMuxStep] = MAGNITUDE;       
      }
      currentMuxStep++; //increment steps of multiplexer

    }
    else{
        Wavegen.setWave(AD9833_OFF);
        if(isInhomogeneous_DONE == true){
          currentState = measurementDONE;

        }
        else{
          currentMuxStep = 0;
          currentState = inHOMOGENEOUS;

        }
    }

  break;
  case inHOMOGENEOUS:
    /*
    currentMuxStep = 0 && MuxSwitchPattern = 0
    Start dma timer
    set currentstate to measuring state 
    */
    Serial.println("HOMOGENEOUS measurement done. Insert object and press button for inHOMOGENEOUS measurement");
    digitalWriteFast(RED_LED, LOW);
    digitalWriteFast(GREEN_LED, HIGH);
    digitalWriteFast(BLUE_LED, LOW);
    strip.clear();
    strip.show();
    digitalWriteFast(RED_LED, LOW);
    digitalWriteFast(GREEN_LED, HIGH);
    digitalWriteFast(BLUE_LED, LOW);
    waitForButtonPress();

    Wavegen.begin();
    Wavegen.setWave(AD9833_SINE);
    Wavegen.setFrequency(F_STIM, 0);
    Wavegen.setFrequencyChannel(0);
    
    isInhomogeneous_DONE = true;
    currentState = MEASURING;
  break;
  case measurementDONE:
    /*
      stop DMA timer
      Print homogenous and inhomogenous data
      current state == IDLE
    */
      
   //print results
  Serial.println("-------------HOMOGENOUS-------------");
  for(int n = 0; n < Num_measurements; n++){
    
    Serial.printf("%.6f\n",homog_data[n]);
  }

  Serial.println("-------------inHOMOGENOUS-------------");
  for(int n = 0; n < Num_measurements; n++){  
    
    Serial.printf("%.6f\n",inhomog_data[n]);
  }
  Serial.println("############################");
  Serial.println("\tmeasurementDONE\t");
  Serial.println("############################");
  strip.clear();
  strip.show();

  digitalWriteFast(RED_LED, HIGH);
  digitalWriteFast(GREEN_LED, LOW);
  digitalWriteFast(BLUE_LED, LOW);
  delay(2000);
  currentState = IDLE;
  break;
  }

}

void SwitchMuxPattern(int step){

  if(step >= Num_measurements)
  {
    return;
  }
  // for adjacent stimmulation pattern [adjacent adjacent]
  //note 208 measurements
  int i = step/(NUM_CHANNELS - 3);
  int j = step%(NUM_CHANNELS - 3);

  int  Chan_pos_InjectNeg = i;
  int Chan_pos_InjectPos = (i + 1)%NUM_CHANNELS;
  int Chan_pos_VmeasPos = (i + 2 + j)%NUM_CHANNELS;
  int Chan_pos_VmeasNeg = (Chan_pos_VmeasPos + 1)%NUM_CHANNELS;
 
  

  // for pseudo-adjacent stimmulation pattern [opposite adjacent]
  //current injectio is in opposite pattern, voltage measuremtn is in adjacent pattern
  //note 192 meas instead of 208
  /*int i = step / (NUM_CHANNELS - 4);
  int j = step % (NUM_CHANNELS - 4);
  int halfway = NUM_CHANNELS / 2;

  int offset = 0;

  // CORRECTED: Match EIDORS [0 8] where '0' is Positive and '8' is Negative
  int Chan_pos_InjectNeg = i;
  int Chan_pos_InjectPos = (i + halfway) % NUM_CHANNELS;

  // Your exact offset logic (Works perfectly to skip injectors)
  if (j < (halfway - 2)) {
    offset = j + 1;
  } else {
    offset = j + 3;
  }

  // Matches EIDORS [0 1] where '0' is V_Pos and '1' is V_Neg
  int Chan_pos_VmeasPos = (i + offset) % NUM_CHANNELS;
  int Chan_pos_VmeasNeg = (Chan_pos_VmeasPos + 1) % NUM_CHANNELS;*/





  //Mux reciprical for reciprocity tests
  /*int Chan_pos_VmeasPos = i;
  int Chan_pos_VmeasNeg = (i + 1)%NUM_CHANNELS;
  int Chan_pos_InjectPos = (i + 2 + j)%NUM_CHANNELS;
  int Chan_pos_InjectNeg = (Chan_pos_InjectPos + 1)%NUM_CHANNELS;*/


  InjectPos.setChannel(Chan_pos_InjectPos);
  InjectNeg.setChannel(Chan_pos_InjectNeg);
  VmeasurePos.setChannel(Chan_pos_VmeasPos);
  VmeasureNeg.setChannel(Chan_pos_VmeasNeg);      
  
  //Serial.printf("STEP: %d ---- CHANNEL SWITCH: %d %d %d %d\n",currentMuxStep,  Chan_pos_InjectPos, Chan_pos_InjectNeg, Chan_pos_VmeasPos, Chan_pos_VmeasNeg);
  /*strip.clear();
  strip.setPixelColor(Chan_pos_InjectPos, 255, 0, 0); //red
  strip.setPixelColor(Chan_pos_InjectNeg, 255, 0, 0);
  strip.setPixelColor(Chan_pos_VmeasPos, 0, 255, 0); // blue
  strip.setPixelColor(Chan_pos_VmeasNeg, 0,255, 0);
  strip.show();*/
}

void waitForButtonPress() {
  // 1. Wait for Press (LOW)
  while(digitalRead(START_BUTTON) == HIGH) {
    // idle loop
  }
  delay(50); // Debounce press

  // 2. Wait for Release (HIGH)
  while(digitalRead(START_BUTTON) == LOW) {
    // wait for user to let go
  }
  delay(50); // Debounce release
}







