#include <Arduino.h>

#define ACCELERATOR_PIN D1
#define MAX_SPEED_PIN   A0
#define DIR_SELECT_PIN  D5

#define MOTOR_DIR_PIN   D6
#define MOTOR_SPEED_PIN D2

// Delay between adjusting speed in milliseconds.
#define RAMP_DELAY      5
#define MAX_MOTOR_SPEED 1022

enum Direction { Forwards, Reverse };
enum RampState { Accelerating, Decelerating };

int currentSpeed = 0;
long int prevNow = millis();

Direction prevDirection = Forwards;
bool changingDir = false;

void setup() {
  Serial.begin(115200);
  pinMode(ACCELERATOR_PIN, INPUT);
  pinMode(DIR_SELECT_PIN, INPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
}

void loop() {
  // Set currentDirection to Forwards or Reverse
  Direction currentDirection = (digitalRead(DIR_SELECT_PIN) == HIGH ? Forwards : Reverse);

  // Has the direction changed since last loop?
  if (currentDirection != prevDirection) {
    changingDir = true;
    prevDirection = currentDirection;
  }

  // Only write to MOTOR_DIR_PIN if we aren't moving
  if (currentSpeed <= 0) {
    changingDir = false;
    digitalWrite(MOTOR_DIR_PIN, (currentDirection == Forwards ? HIGH : LOW));
  }

  // Read max speed from potentiometer
  int maxSpeed = analogRead(MAX_SPEED_PIN);

  // Get current accelerator button state
  int buttonState = digitalRead(ACCELERATOR_PIN);

  // Only if accelerator is pressed and not currently changing direction, set
  // ramp state to accelerating, otherwise we're decelerating
  RampState rampState;
  if (buttonState == HIGH && !changingDir) {
    rampState = Accelerating;
  } else {
    rampState = Decelerating;
  }

  // Get current time in milliseconds
  long int now = millis();

  // If it's been RAMP_DELAY milliseconds since previous time...
  if (now - prevNow >= RAMP_DELAY) {
    // reset previous time to now
    prevNow = now;
    // Increase speed if we're accelerating otherwise decrease
    if (rampState == Accelerating) {
      currentSpeed++;
    } else {
      currentSpeed--;
    }

    // Cap speed to what's selected on the potentiometer
    currentSpeed = constrain(currentSpeed, 0, maxSpeed);
    // Also cap speed to MAX_MOTOR_SPEED
    currentSpeed = constrain(currentSpeed, 0, MAX_MOTOR_SPEED);

    Serial.println(currentSpeed);
  }

  // Send speed to the motor
  analogWrite(MOTOR_SPEED_PIN, currentSpeed);
}
