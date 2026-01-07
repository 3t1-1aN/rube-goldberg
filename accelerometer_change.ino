#include <Servo.h>
#include "notes.h"

// --- IO ---
const int armSensorPin = A1; // Connect X, Y, or Z from accel here
const int reedPin = 2;
const int laserPin = 8;
const int speakerPin = 11;
const int photoPin = A0;

int moveThreshold = 30;     
int lightThreshold = 50;      

int armStartPos = 0;

// --- Servos ---
Servo panServo;
Servo tiltServo;
Servo dropServo;

void setup() {
  Serial.begin(9600);

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

  // --- CALIBRATION STEP ---
  Serial.print("Calibrating Arm Position...");
  long total = 0;
  for(int i=0; i<10; i++){
    total += analogRead(armSensorPin);
    delay(50);
  }
  armStartPos = total / 10;
  
  Serial.print(" Baseline set at: ");
  Serial.println(armStartPos);
  Serial.println("System ready — waiting for ARM MOVEMENT...");
}

void loop() {
  waitForArmMove();
  Serial.println("Arm movement detected! Chain beginning!");
  delay(5000); 
}

void waitForArmMove() {
  int currentVal = 0;
  int diff = 0;

  while (true) {
    currentVal = analogRead(armSensorPin);
    
    // Check difference between NOW and START
    diff = abs(currentVal - armStartPos);

    // Uncomment this line to see the numbers in the Serial Monitor for testing:
    // Serial.println(diff); 

    if (diff > moveThreshold) {
      Serial.println("TRIGGER: Arm moved!");
      delay(100);
      waitForReedTrigger();
      return;
    }
    delay(20);
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
// WAIT FOR LASER HIT
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
