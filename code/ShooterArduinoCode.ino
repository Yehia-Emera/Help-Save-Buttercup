#include <Stepper.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial BT(12, 13);
SoftwareSerial ArduinoComm(A0, A1);

// ----- Stepper -----
#define stepsPerRev 2048
#define movementSteps 100

int currentStep = 0;
Stepper stepperMotor(stepsPerRev, 8, 9, 10, 11);

// ----- Motor (H-Bridge) -----
#define motorEN 5
#define motorIN1 7
#define motorIN2 6

// ----- IR Sensors -----
#define irSensor 2  // shooter IR
#define BoxA_IR 3
#define BoxB_IR 4

bool ballDetected = false;

enum ShootState { IDLE, BACKWARD, PAUSE, FORWARD };
ShootState shootState = IDLE;

unsigned long previousTime = 0;

bool lastSensorState = HIGH;  // shooter IR
bool bluetoothConnected = false;
int healthHearts = 10;

String Question = ""; // Stores current active question text

// Master-Request & Game State Tracking
unsigned long lastPollTime = 0;
const unsigned long pollInterval = 80; 
char lastProcessedColor = '0';

int activeBallColor = 0;      // 0 = None, 1 = Red, 2 = Blue
char correctDirection = ' ';   // Stores 'R' or 'L'
char lastChosenDir = ' ';      // Tracks the user's last mechanical command ('R' or 'L')

// Custom Hearts
byte heartCharacter[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

// Timing
#define backDuration 0
#define pauseDuration 500
#define forwardDuration 175

// -------------------------------------------------------

void updateLCDDisplay();
void stopMotor();
void shoot();

void setup() {
  pinMode(motorEN, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  digitalWrite(motorEN, HIGH);

  pinMode(irSensor, INPUT);
  pinMode(BoxA_IR, INPUT);
  pinMode(BoxB_IR, INPUT);

  Serial.begin(9600);
  BT.begin(9600);
  ArduinoComm.begin(9600);

  stepperMotor.setSpeed(10);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, heartCharacter);

  // Seed random generator using noise from an unconnected pin
  randomSeed(analogRead(A5));

  // Print initial state text
  lcd.setCursor(0, 0);
  lcd.print("Connect Device");
  lcd.setCursor(0, 1);
  lcd.print("Press Any Button To Start");

  BT.listen();
}

void loop() {
  
  // 1. --- Master-Request Pattern: Request color from Main Arduino ---
  if (millis() - lastPollTime >= pollInterval) {
    lastPollTime = millis();
    
    ArduinoComm.listen();   
    ArduinoComm.print('?'); 
    
    unsigned long startTime = millis();
    char incomingColor = 'X';
    
    while (millis() - startTime < 4) {
      if (ArduinoComm.available()) {
        incomingColor = ArduinoComm.read();
        break;
      }
    }
    
    BT.listen(); // Switch back to Bluetooth state immediately
    
    if (incomingColor == '0' || incomingColor == '1' || incomingColor == '2') {
      if (incomingColor != lastProcessedColor) {
        lastProcessedColor = incomingColor;
        
        if (incomingColor == '1') { // RED BALL: Multiplication
          activeBallColor = 1;
          int val1 = random(2, 10);
          int val2 = random(2, 10);
          int rightAns = val1 * val2;
          int wrongAns = rightAns + (random(0, 2) == 0 ? random(3, 6) : -random(3, 6));
          if (wrongAns == rightAns) wrongAns += 4;
          
          // Randomly decide if the right answer is Choice R or Choice L
          bool rightIsR = (random(0, 2) == 0);
          correctDirection = rightIsR ? 'R' : 'L';
          
          Question = "RED BALL: Multiplication Question Generated!\n";
          Question += String(val1) + " x " + String(val2) + " = ?\n";
          if (rightIsR) {
            Question += "-> Choice R : " + String(rightAns) + " | Choice L : " + String(wrongAns);
          } else {
            Question += "-> Choice L : " + String(rightAns) + " | Choice R : " + String(wrongAns);
          }
          
          BT.println("\n========================================");
          BT.println(Question);
          BT.println("========================================");
        } 
        else if (incomingColor == '2') { // BLUE BALL: Addition
          activeBallColor = 2;
          int val1 = random(10, 100);
          int val2 = random(10, 100);
          int rightAns = val1 + val2;
          int wrongAns = rightAns + (random(0, 2) == 0 ? random(5, 12) : -random(5, 12));
          if (wrongAns == rightAns) wrongAns += 10;
          
          // Randomly decide if the right answer is Choice R or Choice L
          bool rightIsR = (random(0, 2) == 0);
          correctDirection = rightIsR ? 'R' : 'L';
          
          Question = "BLUE BALL: Addition Question Generated!\n";
          Question += String(val1) + " + " + String(val2) + " = ?\n";
          if (rightIsR) {
            Question += "-> Choice R : " + String(rightAns) + " | Choice L : " + String(wrongAns);
          } else {
            Question += "-> Choice L : " + String(rightAns) + " | Choice R : " + String(wrongAns);
          }
          
          BT.println("\n========================================");
          BT.println(Question);
          BT.println("========================================");
        }
        else if (incomingColor == '0') {
          // FIX: Do NOT clear activeBallColor or correctDirection here anymore!
          // Let the question variables persist until the user shoots.
        }
      }
    }
  }

  // 2. --- Local IR Sensor Logic ---
  bool sensorState = digitalRead(irSensor);
  if (sensorState == LOW) {
    ballDetected = true;
  }

  if (sensorState != lastSensorState) {
    ArduinoComm.print(sensorState); 
    lastSensorState = sensorState;
  }

  // 3. --- Bluetooth Command Processing ---
  if (BT.available()) {
    char cmd = BT.read();
    if (cmd == '\r' || cmd == '\n' || cmd == ' ') return;

    if (!bluetoothConnected) {
      bluetoothConnected = true;
      lcd.clear(); 
      updateLCDDisplay();
    }

    // Tester Command: Drops health by 2 hearts
    if (cmd == '2') {
      healthHearts -= 2;
      if (healthHearts < 0) healthHearts = 0;
      updateLCDDisplay();
    }
    // Mechanical Movements (Tracks what direction was chosen)
    else if (cmd == 'L' && (currentStep == 0 || currentStep == movementSteps)) {
      lastChosenDir = 'L'; // Save user target choice
      stepperMotor.step(-movementSteps);
      currentStep -= movementSteps;
    } 
    else if (cmd == 'R' && (currentStep == 0 || currentStep == -movementSteps)) {
      lastChosenDir = 'R'; // Save user target choice
      stepperMotor.step(movementSteps);
      currentStep += movementSteps;
    } 
    // Mechanical Firing System (Evaluates dynamic answers upon execution)
    else if (cmd == 'S' && shootState == IDLE && ballDetected) {
      
      // Check if there's an active question awaiting validation
      if (activeBallColor != 0 && correctDirection != ' ') {
        if (lastChosenDir == correctDirection) {
          BT.println("\n>>> CORRECT TARGET HIT! <<<");
          if (activeBallColor == 1) {        // Red Ball Correct
            healthHearts -= 2;
          } else if (activeBallColor == 2) { // Blue Ball Correct
            healthHearts -= 1;
          }
          if (healthHearts < 0) healthHearts = 0;
          updateLCDDisplay();
        } else {
          BT.println("\n>>> WRONG TARGET! No damage dealt. <<<");
        }
        
        // Clear active status here so you can't cheat by shooting the same ball twice
        activeBallColor = 0;
        correctDirection = ' ';
      }

      shootState = BACKWARD;
      previousTime = millis();
    } 
    else if (cmd == 'X') {
      ballDetected = false;
      shootState = IDLE;
      stopMotor();
    }
  }

  // Local Box IR Homing behavior
  if (digitalRead(BoxA_IR) == LOW || digitalRead(BoxB_IR) == LOW) {
    if (currentStep == movementSteps) {
      stepperMotor.step(-movementSteps);
      currentStep -= movementSteps;
    } else if (currentStep == -movementSteps) {
      stepperMotor.step(movementSteps);
      currentStep += movementSteps;
    }
  }

  shoot();
}

void shoot() {
  unsigned long currentTime = millis();
  switch (shootState) {
    case IDLE:
      break;
    case BACKWARD:
      digitalWrite(motorIN1, HIGH);
      digitalWrite(motorIN2, LOW);
      if (currentTime - previousTime >= backDuration) {
        stopMotor();
        shootState = PAUSE;
        previousTime = currentTime;
      }
      break;
    case PAUSE:
      if (currentTime - previousTime >= pauseDuration) {
        shootState = FORWARD;
        previousTime = currentTime;
      }
      break;
    case FORWARD:
      digitalWrite(motorIN1, LOW);
      digitalWrite(motorIN2, HIGH);
      if (currentTime - previousTime >= forwardDuration) {
        stopMotor();
        shootState = IDLE;
        ballDetected = false;
      }
      break;
  }
}

void stopMotor() {
  digitalWrite(motorIN1, LOW);
  digitalWrite(motorIN2, LOW);
}

void updateLCDDisplay() {
  if (!bluetoothConnected) return;

  lcd.setCursor(0, 0);
  lcd.print("Health:         ");  
  lcd.setCursor(0, 1);
  for (int i = 0; i < 10; i++) {
    if (i < healthHearts) {
      lcd.write(byte(0));       
    } else {
      lcd.print(" ");           
    }
  }
}
