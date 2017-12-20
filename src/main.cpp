#include <Arduino.h>

#define ACCELERATOR_PIN D1
#define MAX_SPEED_PIN   A0
#define DIR_SELECT_PIN  D5

#define MOTOR_DIR_PIN   D6
#define MOTOR_SPEED_PIN D2

#define MAX_POT         1024
#define MAX_MOTOR_SPEED 1022

// Delay between adjusting speed in milliseconds.
#define RAMP_INTERVAL   5

enum Direction {
  Forwards,
  Reverse
};

enum RampState {
  Accelerating,
  Decelerating
};

Direction getSelectedDirection();

unsigned int currentSpeed = 0;
unsigned long prevNow = millis();

Direction prevDirection;
bool changingDir = false;

void setup() {
  Serial.begin(115200);

  pinMode(ACCELERATOR_PIN, INPUT);
  pinMode(DIR_SELECT_PIN, INPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);

  prevDirection = getSelectedDirection();
}

void loop() {
  // Set currentDirection to Forwards or Reverse
  Direction currentDirection = getSelectedDirection();

  // Has the direction changed since last loop?
  if (currentDirection != prevDirection) {
    changingDir = true;
  }
  prevDirection = currentDirection;

  // Only write to MOTOR_DIR_PIN if we aren't moving
  if (currentSpeed == 0) {
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
  unsigned long now = millis();

  // If it's been RAMP_INTERVAL milliseconds since previous time...
  if (now - prevNow >= RAMP_INTERVAL) {
    // Reset previous time to now
    prevNow = now;
    // Increase speed if we're accelerating otherwise decrease
    switch (rampState) {
      case Accelerating: currentSpeed++;
      case Decelerating: currentSpeed--;
    }

    // Cap speed to what's selected on the potentiometer
    currentSpeed = constrain(currentSpeed, 0, maxSpeed);
    // Map potentiometer value to the motor value
    currentSpeed = map(currentSpeed, 0, MAX_POT, 0, MAX_MOTOR_SPEED);

    Serial.println(currentSpeed);
  }

  // Send speed to the motor
  analogWrite(MOTOR_SPEED_PIN, currentSpeed);
}

Direction getSelectedDirection() {
  return digitalRead(DIR_SELECT_PIN) != HIGH ? Reverse : Forwards;
}
