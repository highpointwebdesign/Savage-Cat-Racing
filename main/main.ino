#include <Wire.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <stdlib.h>  // For random numbers

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

  // Comment out the MPU6050 initialization test
  /*
  if (!initializeMPU6050()) {
    Serial.println("Critical Error: MPU6050 initialization failed. System halted.");
    enterErrorState();
  }

  Serial.println("MPU6050 initialized successfully. System operational.");
  */

  attachServos();
  testAllServos();  // Test all servos

  analyzeTestResults();
}

void loop() {
  // Main operational code
}

/*
// Comment out the MPU6050 initialization function
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
*/

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

  double initialTilt = getTilt("initial");  // Simulate initial tilt

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

  // Simulate tilt after moving to minimum position
  double minTilt = getTilt(description);

  // Move servos to their maximum positions
  servo1.write(SERVO_MAX);
  servo2.write(SERVO_MAX);
  delay(500);

  // Simulate tilt after moving to maximum position
  double maxTilt = getTilt(description);

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

double getTilt(const char* description) {
  // Generate random mock tilt values
  // Simulate a failure for one specific servo pair
  if (strcmp(description, "Driver Side") == 0) {
    return (rand() % 100) / 10.0;  // Random value between 0.0 and 10.0
  } else if (strcmp(description, "Passenger Side") == 0) {
    return 5.0 + (rand() % 50) / 10.0;  // Random value between 5.0 and 10.0
  } else if (strcmp(description, "Front") == 0) {
    return 0.0;  // Simulate a failure condition with constant tilt
  } else if (strcmp(description, "Rear") == 0) {
    return 5.0 + (rand() % 50) / 10.0;  // Random value between 5.0 and 10.0
  } else {
    return 0.0;  // Default value for initial tilt
  }
}

void analyzeTestResults() {
  if (!driverSideTestPassed) Serial.println("Driver Side test failed.");
  if (!passengerSideTestPassed) Serial.println("Passenger Side test failed.");
  if (!frontTestPassed) Serial.println("Front test failed.");
  if (!rearTestPassed) Serial.println("Rear test failed.");
}
