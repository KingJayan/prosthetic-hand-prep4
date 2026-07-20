/*
  servo tester

  pins:
    wrist  -> D2
    index  -> D4
    middle -> D7
    thumb  -> D8
*/

#include <Servo.h>

const uint8_t NUM_SERVOS = 4;

const uint8_t SERVO_PIN[NUM_SERVOS] = {
  2, 4, 7, 8
};

const uint8_t SERVO_MIN[NUM_SERVOS] = {
  0, 10, 10, 10
};

const uint8_t SERVO_MAX[NUM_SERVOS] = {
  180, 160, 160, 160
};





// 1 = one servo at a time
// 0 = all together
#define TEST_INDIVIDUAL 1

// delay between angle updates (ms)
const uint8_t STEP_DELAY = 15;

// deg per step
const uint8_t STEP_SIZE = 1;

// pause at each end of travel (ms)
const unsigned long END_PAUSE = 500;





Servo servo[NUM_SERVOS];

void moveServo(uint8_t index) {

  Serial.print("Testing Servo ");
  Serial.println(index + 1);

  // sweep up
  for (int angle = SERVO_MIN[index];
       angle <= SERVO_MAX[index];
       angle += STEP_SIZE) {

    servo[index].write(angle);
    delay(STEP_DELAY);
  }

  delay(END_PAUSE);

  // sweep down
  for (int angle = SERVO_MAX[index];
       angle >= SERVO_MIN[index];
       angle -= STEP_SIZE) {

    servo[index].write(angle);
    delay(STEP_DELAY);
  }

  delay(END_PAUSE);
}

void setup() {

  Serial.begin(115200);

  for (uint8_t i = 0; i < NUM_SERVOS; i++) {
    servo[i].attach(SERVO_PIN[i]);
    servo[i].write(SERVO_MIN[i]);
  }

  delay(1000);

  Serial.println("=== Servo Tester ===");

#if TEST_INDIVIDUAL
  Serial.println("Mode: Individual");
#else
  Serial.println("Mode: All Together");
#endif
}

void loop() {

#if TEST_INDIVIDUAL

  for (uint8_t i = 0; i < NUM_SERVOS; i++) {
    moveServo(i);
    delay(500);
  }

#else

  // sweep all up
  for (int angle = 0; angle <= 180; angle += STEP_SIZE) {

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {

      int mapped = map(
        angle,
        0,
        180,
        SERVO_MIN[i],
        SERVO_MAX[i]
      );

      servo[i].write(mapped);
    }

    delay(STEP_DELAY);
  }

  delay(END_PAUSE);

  // sweep all down
  for (int angle = 180; angle >= 0; angle -= STEP_SIZE) {

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {

      int mapped = map(
        angle,
        0,
        180,
        SERVO_MIN[i],
        SERVO_MAX[i]
      );

      servo[i].write(mapped);
    }

    delay(STEP_DELAY);
  }

  delay(END_PAUSE);

#endif
}
