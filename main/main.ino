#include <ESP32Servo.h>
#include <Wire.h>
#include <MPU6050.h>

Servo servo1; // Front Left
Servo servo2; // Front Right
Servo servo3; // Rear Right
Servo servo4; // Rear Left

MPU6050 gyro;

const int servo1Pin = 2;
const int servo2Pin = 3;
const int servo3Pin = 4;
const int servo4Pin = 5;

struct GyroData {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

GyroData baseline, leftSideTest, frontTest, rightSideTest, rearTest;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  gyro.initialize();

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo3.attach(servo3Pin);
  servo4.attach(servo4Pin);

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
  servo4.write(0);
  servo1.write(0);
  delay(500);
  leftSideTest = recordGyroSettings("Left Side Test");
}

void testFront() {
  servo4.write(90);
  servo2.write(0);
  delay(500);
  frontTest = recordGyroSettings("Front Test");
}

void testRightSide() {
  servo1.write(90);
  servo3.write(0);
  delay(500);
  rightSideTest = recordGyroSettings("Right Side Test");
}

void testRear() {
  servo2.write(90);
  servo4.write(0);
  delay(500);
  rearTest = recordGyroSettings("Rear Test");
}

GyroData recordGyroSettings(String step) {
  GyroData data;
  gyro.getMotion6(&data.ax, &data.ay, &data.az, &data.gx, &data.gy, &data.gz);

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
    } else if (gyroXChange < 0) {
      Serial.println("Failure: Servo 1 (front left) did not lower as expected (left tilt detected).");
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
