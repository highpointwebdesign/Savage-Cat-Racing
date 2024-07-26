// This code version includes the MPU6050 initialization, servo attachment, servo testing, error handling, and test result analysis.

#include <Wire.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// Constants and configurations
#define SERVO_MIN 70
#define SERVO_MID 90
#define SERVO_MAX 110

Servo driverFront, driverRear, passengerFront, passengerRear;  // Servo objects
#define DRIVER_FRONT_PIN 5
#define DRIVER_REAR_PIN 19
#define PASSENGER_FRONT_PIN 18
#define PASSENGER_REAR_PIN 21

#define MPU6050_ADDRESS 0x68
#define WHO_AM_I_REGISTER 0x75
#define EXPECTED_WHO_AM_I 0x68  // Expected return value from WHO_AM_I register of MPU6050
#define LED_PIN 2               // Onboard LED pin on ESP32

// LED pins for visual indicators
#define DRIVER_FRONT_LED 12
#define DRIVER_REAR_LED 13
#define PASSENGER_FRONT_LED 14
#define PASSENGER_REAR_LED 15

bool driverSideTestPassed = true;
bool passengerSideTestPassed = true;
bool frontTestPassed = true;
bool rearTestPassed = true;

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Set the LED pin as an output

  // Set LED pins as outputs
  pinMode(DRIVER_FRONT_LED, OUTPUT);
  pinMode(DRIVER_REAR_LED, OUTPUT);
  pinMode(PASSENGER_FRONT_LED, OUTPUT);
  pinMode(PASSENGER_REAR_LED, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  if (!initializeMPU6050()) {
    Serial.println("Critical Error: MPU6050 initialization failed. System halted.");
    enterErrorState();
  }

  Serial.println("MPU6050 initialized successfully. System operational.");

  attachServos();
  testAllServos();  // Test all servos

  analyzeTestResults();
}

void loop() {
  // Main operational code
}

bool initializeMPU6050() {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(WHO_AM_I_REGISTER);  // WHO_AM_I register address
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(MPU6050_ADDRESS, 1)) {
    if (Wire.available()) {
      byte whoAmI = Wire.read();
      return (whoAmI == EXPECTED_WHO_AM_I);
    }
  }
  handleMPU6050Error(); // Handle the error if MPU6050 is not initialized correctly
  return false;
}

void handleMPU6050Error() {
  Serial.println("Error: gyro not detected");
  setServosToMidpoint();
  enterErrorState();
}

void setServosToMidpoint() {
  // Move all servos to their midpoint position
  driverFront.write(SERVO_MID);
  driverRear.write(SERVO_MID);
  passengerFront.write(SERVO_MID);
  passengerRear.write(SERVO_MID);
  Serial.println("All servos moved to midpoint.");
}

void enterErrorState() {
  while (true) {
    for (int i = 0; i < 5; i++) {  // Flash 5 times
      digitalWrite(LED_PIN, HIGH);
      delay(200);               // On for 200 milliseconds
      digitalWrite(LED_PIN, LOW);
      delay(200);               // Off for 200 milliseconds
    }
    delay(2000);  // Pause for 2 seconds
  }
}

void attachServos() {
  driverFront.attach(DRIVER_FRONT_PIN);
  driverRear.attach(DRIVER_REAR_PIN);
  passengerFront.attach(PASSENGER_FRONT_PIN);
  passengerRear.attach(PASSENGER_REAR_PIN);
}

void testAllServos() {
  driverFront.write(SERVO_MID);
  driverRear.write(SERVO_MID);
  passengerFront.write(SERVO_MID);
  passengerRear.write(SERVO_MID);
  delay(500);  // Give some time for servos to reach their positions

  // Record initial tilt
  double initialTilt = getTilt();

  // Test each pair of servos
  driverSideTestPassed = testServoPair(driverFront, driverRear, initialTilt, "Driver Side");
  passengerSideTestPassed = testServoPair(passengerFront, passengerRear, initialTilt, "Passenger Side");
  frontTestPassed = testServoPair(driverFront, passengerFront, initialTilt, "Front");
  rearTestPassed = testServoPair(driverRear, passengerRear, initialTilt, "Rear");

  // Return all servos to middle position after tests
  driverFront.write(SERVO_MID);
  driverRear.write(SERVO_MID);
  passengerFront.write(SERVO_MID);
  passengerRear.write(SERVO_MID);
  Serial.println("All tests completed. Servos returned to neutral position.");
}

bool testServoPair(Servo& servo1, Servo& servo2, double initialTilt, const char* description) {
  Serial.print("Testing servos: ");
  Serial.println(description);

  // Move servos to their minimum positions
  servo1.write(SERVO_MIN);
  servo2.write(SERVO_MIN);
  delay(500);

  // Record tilt after moving to minimum position
  double minTilt = getTilt();

  // Move servos to their maximum positions
  servo1.write(SERVO_MAX);
  servo2.write(SERVO_MAX);
  delay(500);

  // Record tilt after moving to maximum position
  double maxTilt = getTilt();

  // Check if the tilt is in the expected direction
  bool testPassed = false;
  if (strcmp(description, "Driver Side") == 0 && minTilt < initialTilt && maxTilt > initialTilt) {
    Serial.println("Driver Side servos test passed.");
    testPassed = true;
  } else if (strcmp(description, "Passenger Side") == 0 && minTilt > initialTilt && maxTilt < initialTilt) {
    Serial.println("Passenger Side servos test passed.");
    testPassed = true;
  } else if (strcmp(description, "Front") == 0 && minTilt < initialTilt && maxTilt > initialTilt) {
    Serial.println("Front servos test passed.");
    testPassed = true;
  } else if (strcmp(description, "Rear") == 0 && minTilt > initialTilt && maxTilt < initialTilt) {
    Serial.println("Rear servos test passed.");
    testPassed = true;
  } else {
    Serial.print(description);
    Serial.println(" servos test failed.");
    testPassed = false;
  }

  // Set LED indicators
  if (testPassed) {
    setLedIndicator(description, HIGH, LOW);  // Green for success
  } else {
    setLedIndicator(description, LOW, HIGH);  // Red for failure
  }

  return testPassed;
}

void setLedIndicator(const char* description, int greenState, int redState) {
  if (strcmp(description, "Driver Side") == 0) {
    digitalWrite(DRIVER_FRONT_LED, greenState);
    digitalWrite(DRIVER_REAR_LED, greenState);
  } else if (strcmp(description, "Passenger Side") == 0) {
    digitalWrite(PASSENGER_FRONT_LED, greenState);
    digitalWrite(PASSENGER_REAR_LED, greenState);
  } else if (strcmp(description, "Front") == 0) {
    digitalWrite(DRIVER_FRONT_LED, greenState);
    digitalWrite(PASSENGER_FRONT_LED, greenState);
  } else if (strcmp(description, "Rear") == 0) {
    digitalWrite(DRIVER_REAR_LED, greenState);
    digitalWrite(PASSENGER_REAR_LED, greenState);
  }
}

double getTilt() {
  // Implement MPU6050 reading to get tilt angle here
  // This function should return the current tilt angle
  // For simplicity, return a mock value
  return 0.0;
}

void analyzeTestResults() {
  Serial.println("Analyzing test results...");
  if (!driverSideTestPassed) Serial.println("Driver Side test failed.");
  if (!passengerSideTestPassed) Serial.println("Passenger Side test failed.");
  if (!frontTestPassed) Serial.println("Front test failed.");
  if (!rearTestPassed) Serial.println("Rear test failed.");

  if (!frontTestPassed && !driverSideTestPassed) {
    Serial.println("Likely failed servo: Driver Front");
  }
  if (!frontTestPassed && !passengerSideTestPassed) {
    Serial.println("Likely failed servo: Passenger Front");
  }
  if (!driverSideTestPassed && !passengerSideTestPassed) {
    Serial.println("Likely failed servos: Driver Front and Passenger Front");
  }
  if (!rearTestPassed && !driverSideTestPassed) {
    Serial.println("Likely failed servo: Driver Rear");
  }
  if (!rearTestPassed && !passengerSideTestPassed) {
    Serial.println("Likely failed servo: Passenger Rear");
  }
  if (!driverSideTestPassed && !passengerSideTestPassed && !frontTestPassed && !rearTestPassed) {
    Serial.println("Multiple servo failures detected.");
  }
}
