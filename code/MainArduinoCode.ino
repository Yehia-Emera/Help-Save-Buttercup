#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial ShooterComm(A0, A1);

//=============Color Sensor Pin definitions and Variable Declaration==================
#define Sorting_Servo_Pin 7
#define S0 10                    
#define S1 9                     
#define S2 12                    
#define S3 8                     
#define Color_Sensor_Output 11   

Servo Sorter_Servo;
const int sorterServoHomeAngle = 90;
const int sorterServoBlueAngle = (45 - 7);
const int sorterServoRedAngle = (135 + 7);

uint8_t Frequency = 0;
uint8_t Color = 0;
uint8_t R = 0;
uint8_t G = 0;
uint8_t B = 0;

uint8_t lastColorState = 0;

//============Lifter and Turnstile Pin definitions and Variable Declaration==================
#define Lifter_Servo_Pin 5
#define Turnstile_Servo_Pin 6
#define At_Lifter_IR_Sensor 3
#define After_Lifter_IR_Sensor 4

Servo Lifter_Servo;
Servo Turnstile_Servo;

const int LifterHomeAngle = 16;
const int LifterTopAngle = (110);

const int TurnstileHomeAngle = 0;
const int Turnstile_Out_Angle = (125);

int Colour_Read(); 

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(Color_Sensor_Output, INPUT);

  Sorter_Servo.attach(Sorting_Servo_Pin);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);
  ShooterComm.begin(9600);  

  pinMode(At_Lifter_IR_Sensor, INPUT);
  pinMode(After_Lifter_IR_Sensor, INPUT);
  Lifter_Servo.attach(Lifter_Servo_Pin);
  Turnstile_Servo.attach(Turnstile_Servo_Pin);

  Sorter_Servo.write(sorterServoHomeAngle);
  Lifter_Servo.write(LifterHomeAngle);
  Turnstile_Servo.write(Turnstile_Out_Angle);
}

void loop() {
  Colour_Read();

  switch (Color) {
    case 1:
      Sorter_Servo.write(sorterServoBlueAngle);
      break;

    case 2:
      Sorter_Servo.write(sorterServoRedAngle);
      break;

    default:
      break;
  }

  // Handle incoming data requests from Shooter Arduino
  if (ShooterComm.available()) {
    char receivedChar = ShooterComm.read();
    
    // If shooter sends Homing / IR Sensor status updates
    if (receivedChar == '0' || receivedChar == '1') {
      bool shooterIRSensorState = (receivedChar == '1'); 
      
      Serial.print("Shooter IR: ");
      Serial.println(shooterIRSensorState); 
      
      if (!shooterIRSensorState) { 
        Sorter_Servo.write(sorterServoHomeAngle);
        Turnstile_Servo.write(TurnstileHomeAngle);
      }
    } 
    // If shooter requests current Color status
    else if (receivedChar == '?') {
      ShooterComm.print(Color); // Immediately reply with the raw color status (0, 1, or 2)
    }
  }

  //=============Lifter and Turnstile Code===================
  if (digitalRead(At_Lifter_IR_Sensor) == LOW) {
    Lifter_Servo.write(LifterTopAngle);
  }
  if (digitalRead(After_Lifter_IR_Sensor) == LOW) {
    Lifter_Servo.write(LifterHomeAngle);
    Turnstile_Servo.write(Turnstile_Out_Angle);
  }
}

//==========Sorter Function===============
int Colour_Read() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  Frequency = pulseIn(Color_Sensor_Output, LOW);
  R = Frequency;

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  Frequency = pulseIn(Color_Sensor_Output, LOW);
  G = Frequency;

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  Frequency = pulseIn(Color_Sensor_Output, LOW);
  B = Frequency;

  if (70 < R && R < 130 && 165 < G && G < 230 && 130 < B && B < 190) {
    Color = 1;  // Red
  } else if (165 < R && R < 225 && 140 < G && G < 200 && 100 < B && B < 150) {
    Color = 2;  // Blue
  } else {
    Color = 0;  // Unknown/No ball
  }
  return Color;
}
