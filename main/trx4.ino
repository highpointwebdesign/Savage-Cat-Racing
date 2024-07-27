#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>
#include <Preferences.h>

MPU6050 mpu;
Servo frontLeftServo, frontRightServo, rearLeftServo, rearRightServo;
Preferences preferences;

const int channel6Pin = 34; // GPIO pin to read PWM signal from channel 6
const int dialPin = 35;     // GPIO pin connected to the ride height dial (potentiometer)
const int multiplierPin = 36; // GPIO pin connected to the multiplier adjustment (potentiometer)
const int balancePin = 39; // GPIO pin connected to the balance adjustment (potentiometer)
const int rangePin = 32; // GPIO pin connected to the range adjustment (potentiometer)
int midPoint = 90;          // Default mid-point for active suspension (range 0 - 180)
float multiplier = 1.0;     // Default multiplier value (range 0.05 - 2.0)
float balance = 0.0;        // Default balance value (-1.0 to 1.0)
int rangeMin = 0;           // Default range minimum (servo degree limit)
int rangeMax = 180;         // Default range maximum (servo degree limit)

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
  multiplier = preferences.getFloat("multiplier", 1.0);
  balance = preferences.getFloat("balance", 0.0);
  rangeMin = preferences.getInt("rangeMin", 0);
  rangeMax = preferences.getInt("rangeMax", 180);

  // Setup GPIO for reading PWM signal, dial, multiplier, balance, and range adjustment
  pinMode(channel6Pin, INPUT);
  pinMode(dialPin, INPUT);
  pinMode(multiplierPin, INPUT);
  pinMode(balancePin, INPUT);
  pinMode(rangePin, INPUT);
}

void loop() {
  int pwmValue = pulseIn(channel6Pin, HIGH);
  int dialValue = analogRead(dialPin);
  int dialMidPoint = map(dialValue, 0, 4095, 0, 9) * 18; // Mapping dial to steps of 18 (0-180 range)
  int multiplierValue = analogRead(multiplierPin);
  multiplier = map(multiplierValue, 0, 4095, 1, 40) * 0.05; // Mapping to steps of 0.15 (0.05-2.0 range)
  int balanceValue = analogRead(balancePin);
  balance = map(balanceValue, 0, 4095, -5, 5) * 0.2; // Mapping to steps of 0.2 (-1.0 to 1.0 range)
  int rangeValue = analogRead(rangePin);
  rangeMin = map(rangeValue, 0, 4095, 0, 180); // Mapping to range (0-180)
  rangeMax = map(rangeValue, 0, 4095, 0, 180); // Mapping to range (0-180)

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

  // Save settings when active suspension is enabled
  if (pwmValue > 1100 && pwmValue < 1900) {
    preferences.putInt("midPoint", dialMidPoint);
    preferences.putFloat("multiplier", multiplier);
    preferences.putFloat("balance", balance);
    preferences.putInt("rangeMin", rangeMin);
    preferences.putInt("rangeMax", rangeMax);
  }

  delay(100);
}

void setServoPositions(int position) {
  position = constrain(position, rangeMin, rangeMax); // Apply range limits
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

  // Map angles to servo positions (0-180 degrees) and apply multiplier and balance
  int frontPos = midPoint + (map(angleX, -90, 90, -45, 45) * multiplier * (1.0 + balance));
  int rearPos = midPoint + (map(angleX, -90, 90, -45, 45) * multiplier * (1.0 - balance));
  int frontYPos = midPoint + (map(angleY, -90, 90, -45, 45) * multiplier * (1.0 + balance));
  int rearYPos = midPoint + (map(angleY, -90, 90, -45, 45) * multiplier * (1.0 - balance));

  // Apply range limits
  frontPos = constrain(frontPos, rangeMin, rangeMax);
  rearPos = constrain(rearPos, rangeMin, rangeMax);
  frontYPos = constrain(frontYPos, rangeMin, rangeMax);
  rearYPos = constrain(rearYPos, rangeMin, rangeMax);

  // Set servo positions based on tilt, multiplier, balance, and range
  frontLeftServo.write(frontPos);
  frontRightServo.write(frontYPos);
  rearLeftServo.write(rearPos);
  rearRightServo.write(rearYPos);
}
