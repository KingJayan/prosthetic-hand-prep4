/*
  fsr + haptic test

  A4 -> index FSR
  A5 -> middle FSR

  D3 -> index motor
  D5 -> middle motor
  D6 -> palm motor
*/

const uint8_t FSR_PIN[2] = {A4, A5};
const uint8_t VIB_PIN[3] = {3, 5, 6};

const int FSR_THRESHOLD = 120;
const int FSR_CEILING = 800;

const uint8_t VIB_MIN_PWM = 70;
const uint8_t VIB_MAX_PWM = 255;

const unsigned long PRINT_INTERVAL = 100;

unsigned long lastPrint = 0;

int fsrRaw[2];
uint8_t vibDuty[3];

uint8_t forceToDuty(int adc) {

  if (adc < FSR_THRESHOLD)
    return 0;

  int duty = map(
    adc,
    FSR_THRESHOLD,
    FSR_CEILING,
    VIB_MIN_PWM,
    VIB_MAX_PWM
  );

  return constrain(duty, VIB_MIN_PWM, VIB_MAX_PWM);
}

void setup() {

  Serial.begin(115200);

  for (uint8_t i = 0; i < 3; i++) {
    pinMode(VIB_PIN[i], OUTPUT);
    analogWrite(VIB_PIN[i], 0);
  }

  Serial.println("FSR + Haptics Test");
}

void loop() {

  fsrRaw[0] = analogRead(FSR_PIN[0]);
  fsrRaw[1] = analogRead(FSR_PIN[1]);

  vibDuty[0] = forceToDuty(fsrRaw[0]);
  vibDuty[1] = forceToDuty(fsrRaw[1]);

  vibDuty[2] = forceToDuty(max(fsrRaw[0], fsrRaw[1]));

  analogWrite(VIB_PIN[0], vibDuty[0]);
  analogWrite(VIB_PIN[1], vibDuty[1]);
  analogWrite(VIB_PIN[2], vibDuty[2]);

  if (millis() - lastPrint >= PRINT_INTERVAL) {

    lastPrint = millis();

    Serial.print("Index FSR: ");
    Serial.print(fsrRaw[0]);
    Serial.print("  PWM: ");
    Serial.print(vibDuty[0]);

    Serial.print(" | Middle FSR: ");
    Serial.print(fsrRaw[1]);
    Serial.print("  PWM: ");
    Serial.print(vibDuty[1]);

    Serial.print(" | Palm PWM: ");
    Serial.println(vibDuty[2]);
  }
}
