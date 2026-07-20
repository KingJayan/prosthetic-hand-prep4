/* 
 prosthetic arm
 4x servo: wrist rotation + 3 finger knuckles
 4x potentiometers: one per servo, direct manual control
 2x FSR: fingertip grip-force sensing (index + middle)
 3x coin vibration motor via L293D H-bridge for pwm haptics

*/

#include <Servo.h>

const uint8_t NUM_JOINTS = 4;
const uint8_t POT_PIN[NUM_JOINTS] = { A0, A1, A2, A3 };
const uint8_t SERVO_PIN[NUM_JOINTS] = { 2, 4, 7, 8 };
const uint8_t FSR_PIN[2] = { A4, A5 };  // 0 = index tip, 1 = middle tip

// L293D input pins -> vibration motors. must be pwm.
const uint8_t VIB_PIN[3] = { 3, 5, 6 };  // 0 = index, 1 = middle, 2 = palm

/* tuning constants */

const int SERVO_MIN[NUM_JOINTS] = { 5, 5, 5, 5 };
const int SERVO_MAX[NUM_JOINTS] = { 175, 175, 175, 175 };

const int FSR_THRESHOLD = 120;  // ADC counts; below this = "no contact", motors off
const int FSR_CEILING = 800;    // ADC counts treated as "maximum grip force"
//keep threshhold less than cieling

const uint8_t VIB_MIN_PWM = 70;
const uint8_t VIB_MAX_PWM = 255;

// exponential smoothing
// lower ALPHA = smoother but laggy
const float ALPHA = 0.07f;
const uint8_t DEADBAND = 3;  //degrees

const unsigned long PRINT_INTERVAL = 200;

/* global state */
Servo joint[NUM_JOINTS];        // servo objects
float filteredPot[NUM_JOINTS];  // smoothed adc value per pot
int lastAngle[NUM_JOINTS];      // last angle actually written
int fsrRaw[2];                  // raw FSR readings
uint8_t vibDuty[3];             // current PWM duty per motor
unsigned long lastPrint = 0;
unsigned long lastServoUpdate = 0;


void setup() {
  Serial.begin(9600);

  // seed the filter with the current potentiometer position
  for (uint8_t i = 0; i < NUM_JOINTS; i++) {
    joint[i].attach(SERVO_PIN[i]);
    filteredPot[i] = analogRead(POT_PIN[i]);
    lastAngle[i] = potToAngle(i, (int)filteredPot[i]);
    joint[i].write(lastAngle[i]);
  }

  for (uint8_t m = 0; m < 3; m++) {
    pinMode(VIB_PIN[m], OUTPUT);
    analogWrite(VIB_PIN[m], 0);
    vibDuty[m] = 0;
  }

  Serial.println(F("online"));
  Serial.println(F("W_pot I_pot M_pot T_pot | FSR1 FSR2 | V1 V2 V3"));
  delay(300);
}

/* pot ADC -> constrained servo angle helper */
int potToAngle(uint8_t i, int adc) {
  int a = map(adc, 0, 1023, SERVO_MIN[i], SERVO_MAX[i]);
  return constrain(a, SERVO_MIN[i], SERVO_MAX[i]);
}

/* FSR ADC -> haptic PWM duty helper
 - below threshold  -> 0 (motor fully off)
 - above threshold  -> scaled into [VIB_MIN_PWM .. VIB_MAX_PWM] */
uint8_t forceToDuty(int adc) {
  if (adc < FSR_THRESHOLD) return 0;
  int d = map(adc, FSR_THRESHOLD, FSR_CEILING, VIB_MIN_PWM, VIB_MAX_PWM);
  return (uint8_t)constrain(d, VIB_MIN_PWM, VIB_MAX_PWM);
}


int readFilteredADC(uint8_t pin) {
    long sum = 0;
    for (uint8_t i = 0; i < 8; i++) {
        sum += analogRead(pin);
    }
    return sum >> 3;   // divide by 8
}

void loop() {


  if (millis() - lastServoUpdate >= 20) { // 50 Hz
    lastServoUpdate = millis();
    for (uint8_t i = 0; i < NUM_JOINTS; i++) {

      int raw = readFilteredADC(POT_PIN[i]);  // 0..1023

      // exponential moving average: new = a*raw + (1-a)*old.
      filteredPot[i] = (ALPHA * raw) + ((1.0f - ALPHA) * filteredPot[i]);

      int angle = potToAngle(i, (int)filteredPot[i]);

      // deadzone for stutter prevention
      if (abs(angle - lastAngle[i]) >= DEADBAND) {
        angle = constrain(angle, 0, 180);
        joint[i].write(angle);
        lastAngle[i] = angle;
      }
    }
  }


  fsrRaw[0] = analogRead(FSR_PIN[0]);  // index fingertip
  fsrRaw[1] = analogRead(FSR_PIN[1]);  // middle fingertip

  /*haptic feedback with h-bridge*/
  vibDuty[0] = forceToDuty(fsrRaw[0]);  // index motor
  vibDuty[1] = forceToDuty(fsrRaw[1]);  // middle motor

  // palm channel
  int strongest = max(fsrRaw[0], fsrRaw[1]);
  vibDuty[2] = forceToDuty(strongest);  // palm motor
  /*
  for (uint8_t m = 0; m < 3; m++) {
    analogWrite(VIB_PIN[m], vibDuty[m]);
  } */

  if (millis() - lastPrint >= PRINT_INTERVAL) {
    lastPrint = millis();
    /*
    for (uint8_t i = 0; i < NUM_JOINTS; i++) {
      Serial.print(lastAngle[i]);
      Serial.print('\t');
    }
    Serial.print("| ");
    Serial.print(fsrRaw[0]);
    Serial.print('\t');
    Serial.print(fsrRaw[1]);
    Serial.print("\t| ");
    Serial.print(vibDuty[0]);
    Serial.print('\t');
    Serial.print(vibDuty[1]);
    Serial.print('\t');
    Serial.println(vibDuty[2]);
    */
      Serial.print(analogRead(A0));
      Serial.print('\t');
      Serial.print(analogRead(A1));
      Serial.print('\t');
      Serial.print(analogRead(A2));
      Serial.print('\t');
      Serial.println(analogRead(A3));
  }
}
