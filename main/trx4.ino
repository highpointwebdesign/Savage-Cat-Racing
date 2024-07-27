#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>
#include <Preferences.h>

MPU6050 mpu;
Servo frontLeftServo, frontRightServo, rearLeftServo, rearRightServo;
Preferences preferences;

const int channel6Pin = 34; // GPIO pin to read PWM signal from channel 6
const int dialPin = 35;     // GPIO pin connected to the dial (potentiometer)
int midPoint = 90;          // Default mid-point for active suspension

void setup() {
  Serial.begin(115200);

  // Initialize MPU6050
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  // Attach servos to GPIO pins
  frontLeftServo.attach(13);
  frontRightServo.attach(12);
  rearLeftServo.attach(14);
  rearRightServo.attach(27);

  // Initialize Preferences
  preferences.begin("suspension", false);
  midPoint = preferences.getInt("midPoint", 90);

  // Setup GPIO for reading PWM signal and dial
  pinMode(channel6Pin, INPUT);
  pinMode(dialPin, INPUT);
}

void loop() {
  int pwmValue = pulseIn(channel6Pin, HIGH);
  int dialValue = analogRead(dialPin);
  int dialMidPoint = map(dialValue, 0, 4095, 0, 180);

  if (pwmValue < 1100) {
    // Switch in low position: Lower suspension, active suspension off
    setServoPositions(0);
  } else if (pwmValue > 1900) {
    // Switch in high position: Raise suspension to highest, active suspension off
    setServoPositions(180);
  } else {
    // Switch in mid position: Enable active suspension
    midPoint = dialMidPoint;
    enableActiveSuspension();
  }

  // Save dial mid-point when active suspension is enabled
  if (pwmValue > 1100 && pwmValue < 1900) {
    preferences.putInt("midPoint", dialMidPoint);
  }

  delay(100);
}

void setServoPositions(int position) {
  frontLeftServo.write(position);
  frontRightServo.write(position);
  rearLeftServo.write(position);
  rearRightServo.write(position);
}

void enableActiveSuspension() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Calculate tilt angles
  float angleX = atan2(ay, az) * 180 / PI;
  float angleY = atan2(ax, az) * 180 / PI;

  // Map angles to servo positions (0-180 degrees)
  int posX = map(angleX, -90, 90, midPoint - 45, midPoint + 45);
  int posY = map(angleY, -90, 90, midPoint - 45, midPoint + 45);

  // Set servo positions based on tilt
  frontLeftServo.write(posX);
  frontRightServo.write(posX);
  rearLeftServo.write(posY);
  rearRightServo.write(posY);
}
