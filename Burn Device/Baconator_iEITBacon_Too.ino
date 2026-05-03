/*System transferfuntion determination
Run system at a slow PWM of 75%, with a pwm freq of 1Hz
Run the system until the temperature starts to platua
record the values and save it to a csv file

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


double Setpoint = 150.0; //add DiscTempComp to setpoint since center temp lower than et edge temp
double currentTemp;
double correct_temp = 0;
double READY = 0.98*(Setpoint); //turn ready led on, when temp withing 5% of setpoint

//timing
unsigned long previousMillis = 0;
const long interval = 1000; 
double elapsed_time; // Sample time in ms
unsigned long startTime = 0;
double dt;

//PID variables
//slow responce:
/*double Kp = 2.6636;
double Ki = 0.0115;
double Kd = 92.5301;
double pid_filter = 0.0196;*/


//qiock responce
double Kp = 3.0065;
double Ki = 0.01331;
double Kd = 90.6098;
double pid_filter = 0.2635;
double prevError, error, integral;
double derivative = 0;
double prevDeriv = 0;

//slow pwm
unsigned long windowStartTime = 0;
const int WindowSize = 1000; // 500ms slow pwm cycle
unsigned long onTime = 0;


//double INPUT = correct_temp; 
double OUTPUTsig;
const double PWMMin = 0.0;   // Minimum PWM
const double PWMMax = 204; // Maximum PWM (100% power)
int PWMvalue = 0;




//object declarations
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//unsigned long windowStart;

void setup() {
 // Serial.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  
  pinMode(ControlPin, OUTPUT);
  digitalWrite(ControlPin, LOW);

  pinMode(BlueLedPin, OUTPUT);
  pinMode(RedLedPin, OUTPUT);
  pinMode(GreenLedPin, OUTPUT);

  digitalWrite(BlueLedPin, HIGH); 
  digitalWrite(RedLedPin, HIGH); 
  digitalWrite(GreenLedPin, HIGH); 

  /*Serial.print("elapsed_time(s)");
  Serial.print(",");
  Serial.println("PID_correct_temp");*/
  lcd.clear();


  delay(500);

  startTime = millis();
  windowStartTime = millis();

}

void loop() {
  unsigned long currentMillis = millis();

  elapsed_time =  currentMillis - previousMillis;
  if( elapsed_time >= interval) { 


    previousMillis = currentMillis;

    currentTemp = thermocouple.readCelsius();
  
    if (isnan(currentTemp)) {
      //Serial.println("FAULT: Thermocouple read error!");
      digitalWrite(ControlPin, LOW);
      
      return;
    }
    //temperature correction via calibration function
    correct_temp = (((currentTemp - RAW_TEMP_LOW)*REF_RANGE)/RAW_RANGE)+REF_TEMP_LOW;

    //indicate device is on and running
    /*if(correct_temp > 25.0){
      WarningLedState = !WarningLedState;
      digitalWrite(RedLedPin, WarningLedState); 
    }*/

    //PID control implementation
    dt = elapsed_time/1000.0; //pid time in seconds
    error = Setpoint - correct_temp;
    integral += error * dt;
    //Integral Anti-Windup
    //Prevent the integral from accumulating high values
    if(Ki > 0){
        double maxIntegral = PWMMax / Ki;
        if(integral > maxIntegral){
          integral = maxIntegral;
        }
        else if(integral < 0){
          integral = 0; //Heater can't cool, so don't let integral go negative
          }
    }

    derivative = (prevDeriv + (pid_filter * Kd * (error - prevError))) / (1.0 + (pid_filter * dt));;
    prevDeriv = derivative;
    OUTPUTsig     = (Kp*error) + (Ki*integral) + derivative; 
    //NOW convert/clamp OUTPUTsig to a value between PWM bounds --> 8bit value
    PWMvalue = OUTPUTsig;

    if(PWMvalue > PWMMax)
    {
      PWMvalue = PWMMax;
    } 
    else if(PWMvalue < PWMMin)
    {
      PWMvalue = PWMMin;
    }
    prevError  = error;


  double elapsedSeconds = (currentMillis - startTime) / 1000.0;
 // Serial.print("elapsed_time :")
  /*Serial.print(elapsedSeconds);
  Serial.print(",");
  Serial.println(correct_temp);*/
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(correct_temp);

  onTime = map(PWMvalue,0, 255, 0, WindowSize);
  }

  

  if(currentMillis - windowStartTime > WindowSize)
  {
      windowStartTime += WindowSize;
  }

  if(onTime > (currentMillis - windowStartTime)) // 
  {
    digitalWrite(ControlPin, HIGH);
    if(correct_temp >= READY){
       digitalWrite(GreenLedPin, LOW); // Turn LED OFF
    }
    else{
      digitalWrite(BlueLedPin, LOW); // Turn LED ON to visualize the pulsing
    }
    
  } 
  else
  {
    digitalWrite(ControlPin, LOW);

    if(correct_temp >= READY){
           digitalWrite(GreenLedPin, HIGH); // Turn LED OFF
    }
    else
    {
      digitalWrite(BlueLedPin, HIGH); // Turn LED OFF
    }
    
  }




  













  /*
  //BANG BANG control
  if(currentMillis - previousMillis >= interval) { 
    previousMillis = currentMillis;
    currentTemp = thermocouple.readCelsius();
    //temperature correction via calibration function
    correct_temp = (((currentTemp - RAW_TEMP_LOW)*REF_RANGE)/RAW_RANGE)+REF_TEMP_LOW;
    Serial.print("correct TEMP:");
    Serial.println(correct_temp);
    //Hardware Failsafe

    if (isnan(currentTemp)) {
      Serial.println("FAULT: Thermocouple read error!");
      digitalWrite(ControlPin, LOW);
      digitalWrite(BlueLedPin, HIGH); 
      return;
    }
    //Bang-Bang Control
    if(currentTemp < Setpoint) {
       // Too cold -> Turn Heat ON
       digitalWrite(ControlPin, HIGH); 
       digitalWrite(BlueLedPin, LOW); //ON 
    }
    else {
       // Too hot (or exactly on target) -> Turn Heat OFF
       digitalWrite(ControlPin, LOW);  
       digitalWrite(BlueLedPin, HIGH); //OFF
    }
  }
  */
  


}