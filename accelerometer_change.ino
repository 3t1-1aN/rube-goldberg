#include <Servo.h>
#include <Wire.h> 
#include "notes.h"

// --- IO ---
const int MPU_ADDR = 0x68; 
const int reedPin = 2;
const int laserPin = 8;
const int speakerPin = 11;
const int photoPin = A0;


int motionThreshold = 3000;     
int lightThreshold = 50;      

// --- Variables ---
int16_t baseX, baseY, baseZ;

// --- Servos ---
Servo panServo;
Servo tiltServo;
Servo dropServo;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Wake up MPU6050 (it sleeps by default)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  pinMode(laserPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(reedPin, INPUT_PULLUP);

  panServo.attach(3);
  tiltServo.attach(5);
  dropServo.attach(6);

  // Start servos neutral
  panServo.write(180);
  tiltServo.write(0);
  dropServo.write(0);

  // --- CALIBRATION ---
  Serial.println("Calibrating MPU6050 (Don't move the arm!)...");
  delay(1000); 

  
  readMPU(baseX, baseY, baseZ);
  
  Serial.print("Baseline -> X:"); Serial.print(baseX);
  Serial.print(" Y:"); Serial.print(baseY);
  Serial.print(" Z:"); Serial.println(baseZ);
  Serial.println("System ready — waiting for ARM MOVEMENT...");
}

void loop() {
  // STEP 1 — Wait for arm motion
  waitForMotion();
  Serial.println("Movement detected! Chain beginning!");
  

  delay(2000); 
}

// --------------------------------------------------
// READ MPU Helper Function
// --------------------------------------------------
void readMPU(int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  
  // Request 6 bytes (2 for X, 2 for Y, 2 for Z)
  Wire.requestFrom(MPU_ADDR, 6, true); 
  
  x = Wire.read() << 8 | Wire.read(); 
  y = Wire.read() << 8 | Wire.read(); 
  z = Wire.read() << 8 | Wire.read(); 
}

// --------------------------------------------------
// WAIT FOR MOTION
// --------------------------------------------------
void waitForMotion() {
  int16_t currX, currY, currZ;
  long totalDiff;

  while (true) {
    readMPU(currX, currY, currZ);

    // Calculate how different the current position is from the baseline
    // We add up the differences of all 3 axes
    totalDiff = abs(currX - baseX) + abs(currY - baseY) + abs(currZ - baseZ);

    // Debugging: Uncomment to see the numbers
    // Serial.println(totalDiff);

    if (totalDiff > motionThreshold) {
      Serial.println("TRIGGER: Arm moved!");
      delay(100); 
      waitForReedTrigger();
      return;
    }
    delay(50); // Check 20 times a second
  }
}

// --------------------------------------------------
// PAN/TILT SEQUENCE
// --------------------------------------------------
void performPanTiltSearch() {
  playMidi(11, midi1, ARRAY_LEN(midi1));
  for (int p = 180; p > 95; p--) {
    panServo.write(p);
    delay(20);
  }

  for (int t = 0; t < 70; t++) {
    tiltServo.write(t);
    delay(20);
  }
}

// --------------------------------------------------
// WAIT FOR REED SWITCH TRIGGER
// --------------------------------------------------
void waitForReedTrigger() {
  Serial.println("Waiting for magnet / reed switch...");
  performPanTiltSearch();
  Serial.println(digitalRead(reedPin));
  
  while (digitalRead(reedPin) == LOW) {
    delay(5);
    waitForLaserHit();
  }
}

// --------------------------------------------------
// LASER
// --------------------------------------------------
void activateLaser() {
  digitalWrite(laserPin, HIGH);
  delay(1000);
  digitalWrite(laserPin, LOW);
  delay(500);
}

// --------------------------------------------------
// WAIT FOR LASER HIT (photoresistor)
// --------------------------------------------------
void waitForLaserHit() {
  Serial.println("Waiting for laser hit...");
  activateLaser();

  while (analogRead(photoPin) < lightThreshold) {
    delay(10);
  }
  delay(10);
  releaseMarble();
}

// --------------------------------------------------
// MARBLE DROP
// --------------------------------------------------
void releaseMarble() {
  dropServo.write(90);
  delay(1000);
  dropServo.write(0);
  delay(1000);
  Serial.println("Done!");
  return;
}
