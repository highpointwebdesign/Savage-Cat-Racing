#include <Wire.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// Constants and configurations
#define SERVO_MIN 10
#define SERVO_MID 90
#define SERVO_MAX 170

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

  driverFront.write(SERVO_MIN);
delay(250);
  driverRear.write(SERVO_MIN);
  delay(250);
  passengerFront.write(SERVO_MIN);
  delay(250);
  passengerRear.write(SERVO_MIN);
delay(500);
  driverFront.write(SERVO_MAX);
  delay(250);
  driverRear.write(SERVO_MAX);
  delay(250);
  passengerFront.write(SERVO_MAX);
  delay(250);
  passengerRear.write(SERVO_MAX);
delay(500);
  driverFront.write(SERVO_MID);
  delay(250);
  driverRear.write(SERVO_MID);
  delay(250);
  passengerFront.write(SERVO_MID);
  delay(250);
  passengerRear.write(SERVO_MID);
  Serial.println("Testing complete");
  // Assuming getTilt() and testServoPair() are defined elsewhere
  // If you need implementations for these functions, let me know
}

void analyzeTestResults() {
  if (!driverSideTestPassed) Serial.println("Driver Side test failed.");
  if (!passengerSideTestPassed) Serial.println("Passenger Side test failed.");
  if (!frontTestPassed) Serial.println("Front test failed.");
  if (!rearTestPassed) Serial.println("Rear test failed.");
}
