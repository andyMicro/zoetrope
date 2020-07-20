#include <TimeLib.h>

/*
   ______     ______     ______     ______   ______     ______     ______   ______
  /\___  \   /\  __ \   /\  ___\   /\__  _\ /\  == \   /\  __ \   /\  == \ /\  ___\
  \/_/  /__  \ \ \/\ \  \ \  __\   \/_/\ \/ \ \  __<   \ \ \/\ \  \ \  _-/ \ \  __\
    /\_____\  \ \_____\  \ \_____\    \ \_\  \ \_\ \_\  \ \_____\  \ \_\    \ \_____\
    \/_____/   \/_____/   \/_____/     \/_/   \/_/ /_/   \/_____/   \/_/     \/_____/

  MOO2 Zoetrope Unified code for motor and strobe
  July 30th 2018 Labour Robb Godshaw Robb@robb.cc
  
  Distributed under MIT License

  BOM--
  ClearPath MCVC CPM-MCVC-3421S-RLN to be programmed with Zoetrope-MicroMoo2-r2-fromTeknic.mtr
  COntrol cable
  Mosfet ARRAy on cathode of 48v LED bus


  Code controls motor states, timing, led strobes.


*/


#define fastDuration 30 //seconds to spin fast, both the duration and the interval
#define slowDuration 30 //seconds to spin slow, both the duration and the interval

#define forFilming LOW ///or HIGH - turns off strobe mode for frame-mathced filming 30 FPS

#define FPS 44 //frames per second
#define MOTOR_RPM 110// desired FPS / FPZoe (24 for us) *60 (MUST CHANGE WITH PC and Clearpath MSP!!) this variable is a note only.

#define POT_LOW_PIN A9
#define POT_WIPER_PIN A8
#define POT_HIGH_PIN A7

#define CLEARPATH_COMM_PIN GND //BLack wire //not used in code, note only
#define CLEARPATH_EN_PIN 0 //BLUE WIRE
#define CLEARPATH_INPUT_A_PIN 2 //BLUE WIRE
#define CLEARPATH_INPUT_B_PIN 1 //BLUE WIRE
#define CLEARPATH_HLFB_PIN 3 //BLUE WIRE

#define STROBE_TRIGGER_PIN 4//30
#define STROBE_RESOLUTION 14

//#define BRIGHTNESS 7

#define STROBE_DUTY_BRIGHTNESS_ANIM 210//BRIGHTNESS*300/10//140
#define STROBE_DUTY_BRIGHTNESS_ATTRACT 910//BRIGHTNESS*1300/10//1400


#define STOBE_MIN_DUTY 0
#define STROBE_MAX_DUTY 140

#define STROBE_FREQ_ANIM FPS
#define STROBE_FREQ_ATTRACT 29296.875

#define led 13


int ledFreq;
int ledDuty;


// TODO: Why are we using the time library when 24hr time is irrelevant?
// should simplify to using millis() to avoid dependence on external libraries.
// - andy july 2020
unsigned long currentUNIX;//current time in seconds since 1970, which we define as power-up due to not having a 3v battery
unsigned long previousTimeToTurnSlow = fastDuration;//calculated at transition, the time to stop being fast
unsigned long previousTimeToTurnFast = fastDuration  + slowDuration;//calculated at transition, the time to stop being fast

int speedState = LOW;//low for slow, high for fast


void setup()
{
  //  Serial.begin(1000000);
  setTime(0);//sets system time to jan 1 1970 when powered up
  pinMode(led, OUTPUT);//indicator LED
  pinMode(CLEARPATH_EN_PIN, OUTPUT);//indicator LED
  pinMode(CLEARPATH_INPUT_A_PIN, OUTPUT);//indicator LED
  pinMode(CLEARPATH_INPUT_B_PIN, OUTPUT);//indicator LED
  digitalWrite(CLEARPATH_EN_PIN, HIGH); //ENABLE MOTOR

  pinMode(POT_HIGH_PIN, OUTPUT);//indicator LED
  digitalWrite(POT_HIGH_PIN, HIGH); //ENABLE MOTOR
  pinMode(POT_LOW_PIN, OUTPUT);//indicator LED
  digitalWrite(POT_LOW_PIN, LOW); //ENABLE MOTOR

  ledFreq = STROBE_FREQ_ANIM;
  analogWriteResolution(STROBE_RESOLUTION);  // DEFAULT is 8 analogWrite value 0 to 4095, or 4096 for high
  analogWriteFrequency(STROBE_TRIGGER_PIN, STROBE_FREQ_ANIM);
  analogWrite(STROBE_TRIGGER_PIN, ledDuty);
  fastModeSetup();
  if (forFilming == HIGH) filmModeSetup();
}


void loop() {
  currentUNIX = now();

  if (forFilming == HIGH) filmModeLoop();
  else {
    if (speedState == LOW) {
      if (currentUNIX - previousTimeToTurnSlow >= slowDuration) {
        previousTimeToTurnFast = currentUNIX;
        speedState = HIGH;
        fastModeSetup();
      }
      else slowModeLoop();
    }
    else if (speedState == HIGH) {
      if (currentUNIX - previousTimeToTurnFast >= fastDuration) {
        previousTimeToTurnSlow = currentUNIX;
        speedState = LOW;
        slowModeSetup();
      }
      else fastModeLoop();
    }

  }
}

void fastModeSetup() {
  //ledDuty = // (adjBrightness()*STROBE_DUTY_BRIGHTNESS_ANIM)/1000;//map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_MAX_DUTY, STOBE_MIN_DUTY);
  ledDuty = map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_DUTY_BRIGHTNESS_ANIM / 2, STROBE_DUTY_BRIGHTNESS_ANIM * 3);
  analogWriteFrequency(STROBE_TRIGGER_PIN, STROBE_FREQ_ANIM);
  digitalWrite(led, HIGH);
  analogWrite(STROBE_TRIGGER_PIN, ledDuty);
  digitalWrite(CLEARPATH_EN_PIN, HIGH); //ENABLE MOTOR

  digitalWrite(CLEARPATH_INPUT_A_PIN, HIGH); //MOTOR FAST
  digitalWrite(CLEARPATH_INPUT_B_PIN, LOW); //MOTOR SLOW


}
void fastModeLoop() {
  //PASS
}

void slowModeSetup() {
  //  ledDuty =  STROBE_DUTY_BRIGHTNESS_ATTRACT;//map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_MAX_DUTY, STOBE_MIN_DUTY);
  ledDuty = map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_DUTY_BRIGHTNESS_ATTRACT, STROBE_DUTY_BRIGHTNESS_ATTRACT * 3);

  analogWriteFrequency(STROBE_TRIGGER_PIN, STROBE_FREQ_ATTRACT);
  digitalWrite(led, LOW);
  digitalWrite(CLEARPATH_EN_PIN, LOW); //DISABLE MOTOR

  analogWrite(STROBE_TRIGGER_PIN, ledDuty);
  //  digitalWrite(CLEARPATH_INPUT_A_PIN, LOW); //MOTOR SLOW
  //  digitalWrite(CLEARPATH_INPUT_B_PIN, LOW); //MOTOR SLOW


}
void slowModeLoop() {
  //    ledDuty = map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_MAX_DUTY, STOBE_MIN_DUTY);
  //    Serial.println(ledDuty);
  //      analogWrite(STROBE_TRIGGER_PIN, ledDuty);

}
void filmModeSetup() {
  //ledDuty =  (adjBrightness()*STROBE_DUTY_BRIGHTNESS_ATTRACT)/1000; //map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_MAX_DUTY, STOBE_MIN_DUTY);
  ledDuty = map(analogRead(POT_WIPER_PIN), 20, 1023, STROBE_DUTY_BRIGHTNESS_ATTRACT / 2, STROBE_DUTY_BRIGHTNESS_ATTRACT * 3);

  analogWriteFrequency(STROBE_TRIGGER_PIN, STROBE_FREQ_ATTRACT);
  digitalWrite(led, LOW);
  analogWrite(STROBE_TRIGGER_PIN, ledDuty);
  digitalWrite(CLEARPATH_EN_PIN, HIGH); //ENABLE MOTOR
  digitalWrite(CLEARPATH_INPUT_A_PIN, LOW); //MOTOR SLOW
  digitalWrite(CLEARPATH_INPUT_B_PIN, HIGH); //MOTOR SLOW

}
void filmModeLoop() {
  //PASS
}

