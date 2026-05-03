/*System transferfuntion determination
Run system at a slow PWM of 25%, with a pwm freq of 1Hz
Run the system until the temperature starts to platua
record the values and save it to a csv file

implement slow pwm (25% duty cycle at Freq = 1Hz)
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


*/

#include <max6675.h>
#include <math.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- Pin Definitions ---
const int thermoDO = 12;
const int thermoCS = 10;
const int thermoCLK = 13;

const int ControlPin = 9;
const int BlueLedPin = 16; 
const int RedLedPin = 15; 
const int GreenLedPin = 17; 

bool WarningLedState = HIGH;

//From calibration procedure
/*
CorrectedValue = m * RawValue + c _____________________________[1]
m = (ReferenceHigh−ReferenceLow) / RawHigh−RawLow) --> slope ___[2]

c = ReferenceLow − m * RawLow __[3]

substituing m & c into [1]; the final result is;

correct_temp = (((currentTemp - RAW_TEMP_LOW)*REF_RANGE)/RAW_RANGE)+REF_TEMP_LOW;


*/
double RAW_TEMP_HIGH = 101.56;
double RAW_TEMP_LOW = 10.74;
double RAW_RANGE = RAW_TEMP_HIGH - RAW_TEMP_LOW;

double REF_TEMP_HIGH = 86.3;
double REF_TEMP_LOW = 0.5;
double REF_RANGE = REF_TEMP_HIGH - REF_TEMP_LOW;

double DiscTempComp = 0; //compensate for temperature measured at egde, 
double Setpoint = 100.0;//+ DiscTempComp; //add DiscTempComp to setpoint since center temp lower than et edge temp
double currentTemp;
double correct_temp = 0.0;

//Slow pwm setup

const int StepPWM = 128; //50% duty
 

//timing
unsigned long previousMillis = 0;
const long interval = 1000; //1 second interval to log data
double elapsed_time; // Sample time in ms
unsigned long startTime = 0;

// --- Slow PWM Variables ---
unsigned long windowStartTime = 0;
const int WindowSize = 1000; // 1 second Time Proportioning window
unsigned long onTime;



//double INPUT = correct_temp; 
double OUTPUTsig;
const double PWMMin = 0.0;   // Minimum PWM
const double PWMMax = 255.0; // Maximum PWM
int PWMvalue = 0;
//object declarations
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//Setpoint, correct_temp, OUTPUT; 
//PID myPID(&correct_temp, &OUTPUTsig, &Setpoint, Kp, Ki, Kd, DIRECT);

//unsigned long windowStart;

void setup() {

  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  Serial.begin(9600);
  
  pinMode(ControlPin, OUTPUT);
  digitalWrite(ControlPin, LOW);

  pinMode(BlueLedPin, OUTPUT);
  pinMode(RedLedPin, OUTPUT);
  pinMode(GreenLedPin, OUTPUT);

  digitalWrite(BlueLedPin, HIGH); 
  digitalWrite(RedLedPin, HIGH); 
  digitalWrite(GreenLedPin, HIGH); 

  // Pre-calculate how many milliseconds the MOSFET should be ON per window

 /* long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;*/
 //      (64-0) * (1000-0)/(255-0) + 0
  onTime = map(StepPWM, PWMMin, PWMMax, 0, WindowSize); //map pwm value ito window size


  /*Serial.print("----------onTime-----------------:    ");
  Serial.println(onTime);*/

  Serial.print("TIME(s)");
  Serial.print(",");
  Serial.println("Temperature(degC)");

  lcd.clear();

  delay(1500);
  // Synchronize timers
  startTime = millis();
  windowStartTime = millis();

  
}

void loop() {
  unsigned long currentMillis = millis();

  elapsed_time =  currentMillis - previousMillis;
  if( elapsed_time >= interval){ 

    previousMillis = currentMillis;

    currentTemp = thermocouple.readCelsius();
  
    /*if (isnan(currentTemp)){
      Serial.println("FAULT: Thermocouple read error!");
      digitalWrite(ControlPin, LOW);
      return;
    }*/
    //temperature correction via calibration function
    correct_temp = (((currentTemp - RAW_TEMP_LOW)*REF_RANGE)/RAW_RANGE)+REF_TEMP_LOW;
    //correct_temp = 0.9356*(currentTemp) + 15.321;//polinomial 3-order
    /*if(correct_temp > 25.0){
      WarningLedState = !WarningLedState;
      digitalWrite(RedLedPin, WarningLedState);      
    }*/  

    //set MOSFET pin
    //analogWrite(ControlPin,PWMvalue);
    double elapsedSeconds = (currentMillis - startTime) / 1000.0;
   // Serial.print("T: ");
    Serial.print(elapsedSeconds);
    Serial.print(",");
    /*Serial.print("  measuredtemp:");
    Serial.print(currentTemp);
    Serial.print(",");*/
    //Serial.print("  Calibtemp:");
    Serial.println(correct_temp);

    lcd.setCursor(0, 0);
    lcd.print("TEMP: ");
    lcd.print(correct_temp);

  }

  if(currentMillis - windowStartTime > WindowSize){
    windowStartTime += WindowSize;
  }

  if(onTime > (currentMillis - windowStartTime)) // 
  {
    digitalWrite(ControlPin, HIGH);
    digitalWrite(GreenLedPin, LOW); // Turn LED ON to visualize the pulsing
  } 
  else{
    digitalWrite(ControlPin, LOW);
    digitalWrite(GreenLedPin, HIGH); // Turn LED OFF
  }

}