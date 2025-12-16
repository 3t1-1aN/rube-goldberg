#include <Servo.h>
#include "notes.h"

// --- IO ---
const int soundPin = A1;
const int reedPin = 2;
const int laserPin = 8;
const int speakerPin = 11;
const int photoPin = A0;

// --- Thresholds ---
int soundThreshold = 100;     
int lightThreshold = 50;      

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

  Serial.println("System ready — waiting for SNAP...");
}

void loop() {

  // STEP 1 — Wait for snap
  waitForSound();
  Serial.println("Snap detected! chain beginning!");

}



// --------------------------------------------------
// WAIT FOR SNAP SOUND
// --------------------------------------------------
void waitForSound() {
  int soundValue = 0;

  Serial.println("Listening for sound...");
  Serial.println(analogRead(soundPin));

  while (true) {
    soundValue = analogRead(soundPin);
    if (soundValue > soundThreshold) {
      delay(100);
      waitForReedTrigger();
      return;
    }
    delay(10);
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
