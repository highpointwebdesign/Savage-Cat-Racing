#include <ESP32Servo.h>

// Servo objects
Servo servoA;
Servo servoB;
Servo servoC;
Servo servoD;

int posMid = 90;  // Mid point position
int posLow = 0;   // Low point position
int posHigh = 180; // High poit position

int ledPin = 2;   // Onboard LED

void setup() {
  Serial.begin(115200);
  
  // Attach servos to pins
  servoA.attach(13);
  servoB.attach(12);
  servoC.attach(14);
  servoD.attach(27);

  // Initialize servos to mid point
  servoA.write(posMid);
  servoB.write(posMid);
  servoC.write(posMid);
  servoD.write(posMid);
  
  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  
  // Perform tests
  bool testsPassed = performTests();

  if (testsPassed) {
    happyDance();
  } else {
    signalFailure();
  }
}

void loop() {
  // No repeated actions needed in loop
}

bool performTests() {
  // Test case 1
  servoA.write(posLow);
  servoB.write(posLow);
  delay(500);
  if (!checkGyro()) {
    return false;
  }
  returnHome();

  // Test case 2
  servoB.write(posLow);
  servoC.write(posLow);
  delay(500);
  if (!checkGyro()) {
    return false;
  }
  returnHome();

  // Test case 3
  servoC.write(posLow);
  servoD.write(posLow);
  delay(500);
  if (!checkGyro()) {
    return false;
  }
  returnHome();

  // Test case 4
  servoD.write(posLow);
  servoA.write(posLow);
  delay(500);
  if (!checkGyro()) {
    return false;
  }
  returnHome();

  return true;
}

bool checkGyro() {
  // Placeholder for checking the gyro value
  // Replace with actual gyro check code if needed
  // Currently always returns true for simplicity
  return true;
}

void returnHome() {
  servoA.write(posMid);
  servoB.write(posMid);
  servoC.write(posMid);
  servoD.write(posMid);
  delay(2000);
}

void happyDance() {
  for (int i = 0; i < 2; i++) {
    servoA.write(posLow);
    servoB.write(posLow);
    servoC.write(posLow);
    servoD.write(posLow);
    delay(500);
    servoA.write(posHigh);
    servoB.write(posHigh);
    servoC.write(posHigh);
    servoD.write(posHigh);
    delay(500);
    returnHome();
  }
}

void signalFailure() {
  while (true) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      delay(200);
    }
    delay(2000);
  }
}
