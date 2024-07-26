#include <ESP32Servo.h>
#include <Wire.h>
// #include <MPU6050.h> // Commenting out the actual gyro library

Servo servo1; // Front Left
Servo servo2; // Front Right
Servo servo3; // Rear Right
Servo servo4; // Rear Left

// MPU6050 gyro; // Commenting out the actual gyro initialization

const int servo1Pin = 2;
const int servo2Pin = 3;
const int servo3Pin = 4;
const int servo4Pin = 5;

struct GyroData {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

GyroData baseline, leftSideTest, frontTest, rightSideTest, rearTest;

// Mocking defective servos (true means defective)
bool mockDefectiveServo1 = false;
bool mockDefectiveServo2 = false;
bool mockDefectiveServo3 = false;
bool mockDefectiveServo4 = false;

void setup() {
  Serial.begin(115200);
  // Wire.begin();
  // gyro.initialize(); // Commenting out the actual gyro initialization

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  servo4.attach(servo4Pin);

  // Mocking defective servos (You can change these values for testing)
  mockDefectiveServo1 = false;
  mockDefectiveServo2 = true; // For example, servo 2 is defective
  mockDefectiveServo3 = false;
  mockDefectiveServo4 = false;

  initializeServos();
  delay(500);
  baseline = recordGyroSettings("Baseline");

  testLeftSide();
  testFront();
  testRightSide();
  testRear();

  analyzeResults();
}

void loop() {
  // This is intentionally left empty.
}

void initializeServos() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
}

void testLeftSide() {
  servo4.write(mockDefectiveServo4 ? 90 : 0); // Mock defective servo 4 by keeping it at midpoint
  servo1.write(mockDefectiveServo1 ? 90 : 0); // Mock defective servo 1 by keeping it at midpoint
  delay(500);
  leftSideTest = recordGyroSettings("Left Side Test");
}

void testFront() {
  servo4.write(90);
  servo2.write(mockDefectiveServo2 ? 90 : 0); // Mock defective servo 2 by keeping it at midpoint
  delay(500);
  frontTest = recordGyroSettings("Front Test");
}

void testRightSide() {
  servo1.write(90);
  servo3.write(mockDefectiveServo3 ? 90 : 0); // Mock defective servo 3 by keeping it at midpoint
  delay(500);
  rightSideTest = recordGyroSettings("Right Side Test");
}

void testRear() {
  servo2.write(90);
  servo4.write(mockDefectiveServo4 ? 90 : 0); // Mock defective servo 4 by keeping it at midpoint
  delay(500);
  rearTest = recordGyroSettings("Rear Test");
}

GyroData recordGyroSettings(String step) {
  GyroData data;

  // Simulating gyro readings based on mock defects
  if (step == "Left Side Test") {
    data.ax = mockDefectiveServo1 || mockDefectiveServo4 ? 0 : 0; // No X change expected
    data.ay = mockDefectiveServo1 ? 100 : (mockDefectiveServo4 ? -100 : 0);
    data.az = 0;
  } else if (step == "Front Test") {
    data.ax = mockDefectiveServo2 ? 100 : 0;
    data.ay = 0; // No Y change expected
    data.az = 0;
  } else if (step == "Right Side Test") {
    data.ax = 0; // No X change expected
    data.ay = mockDefectiveServo2 ? 100 : (mockDefectiveServo3 ? -100 : 0);
    data.az = 0;
  } else if (step == "Rear Test") {
    data.ax = mockDefectiveServo3 ? 100 : (mockDefectiveServo4 ? -100 : 0);
    data.ay = 0; // No Y change expected
    data.az = 0;
  } else {
    data.ax = 0;
    data.ay = 0;
    data.az = 0;
  }

  data.gx = 0;
  data.gy = 0;
  data.gz = 0;

  Serial.print(step + " - ");
  Serial.print("Accel: ");
  Serial.print("X: "); Serial.print(data.ax); 
  Serial.print(" Y: "); Serial.print(data.ay); 
  Serial.print(" Z: "); Serial.println(data.az);

  Serial.print("Gyro: ");
  Serial.print("X: "); Serial.print(data.gx); 
  Serial.print(" Y: "); Serial.print(data.gy); 
  Serial.print(" Z: "); Serial.println(data.gz);

  return data;
}

void analyzeResults() {
  analyzeGyroData("Left Side Test", baseline, leftSideTest, "left");
  analyzeGyroData("Front Test", baseline, frontTest, "front");
  analyzeGyroData("Right Side Test", baseline, rightSideTest, "right");
  analyzeGyroData("Rear Test", baseline, rearTest, "rear");
}

void analyzeGyroData(String step, GyroData baseline, GyroData current, String side) {
  Serial.println(step + " Analysis:");

  int16_t accelXChange = current.ax - baseline.ax;
  int16_t accelYChange = current.ay - baseline.ay;
  int16_t accelZChange = current.az - baseline.az;
  
  int16_t gyroXChange = current.gx - baseline.gx;
  int16_t gyroYChange = current.gy - baseline.gy;
  int16_t gyroZChange = current.gz - baseline.gz;

  Serial.print("Accel X Change: "); Serial.println(accelXChange);
  Serial.print("Accel Y Change: "); Serial.println(accelYChange);
  Serial.print("Accel Z Change: "); Serial.println(accelZChange);
  
  Serial.print("Gyro X Change: "); Serial.println(gyroXChange);
  Serial.print("Gyro Y Change: "); Serial.println(gyroYChange);
  Serial.print("Gyro Z Change: "); Serial.println(gyroZChange);

  // Analysis logic
  if (side == "left") {
    if (gyroXChange != 0) {
      Serial.println("Failure: Unexpected X tilt when testing left side.");
    }
    if (gyroYChange > 0) {
      Serial.println("Failure: Servo 1 (front left) did not lower as expected (forward tilt detected).");
    } else if (gyroYChange < 0) {
      Serial.println("Failure: Servo 4 (rear left) did not lower as expected (backward tilt detected).");
    } else {
      Serial.println("Left side servos are functioning correctly.");
    }
  } else if (side == "front") {
    if (gyroYChange != 0) {
      Serial.println("Failure: Unexpected Y tilt when testing front side.");
    }
    if (gyroXChange > 0) {
      Serial.println("Failure: Servo 2 (front right) did not lower as expected (right tilt detected).");
    } else {
      Serial.println("Front servos are functioning correctly.");
    }
  } else if (side == "right") {
    if (gyroXChange != 0) {
      Serial.println("Failure: Unexpected X tilt when testing right side.");
    }
    if (gyroYChange > 0) {
      Serial.println("Failure: Servo 2 (front right) did not lower as expected (forward tilt detected).");
    } else if (gyroYChange < 0) {
      Serial.println("Failure: Servo 3 (rear right) did not lower as expected (backward tilt detected).");
    } else {
      Serial.println("Right side servos are functioning correctly.");
    }
  } else if (side == "rear") {
    if (gyroYChange != 0) {
      Serial.println("Failure: Unexpected Y tilt when testing rear side.");
    }
    if (gyroXChange > 0) {
      Serial.println("Failure: Servo 3 (rear right) did not lower as expected (right tilt detected).");
    } else if (gyroXChange < 0) {
      Serial.println("Failure: Servo 4 (rear left) did not lower as expected (left tilt detected).");
    } else {
      Serial.println("Rear servos are functioning correctly.");
    }
  }
}
