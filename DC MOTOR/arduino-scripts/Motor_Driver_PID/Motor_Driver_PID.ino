/* This sketch supports NILab Project FacePush. It has the following features:
 * - Leonardo receives message from Adafruit Feather M0 through I2C communication.
 * - uses Arduino Leonardo and Monster Motor Shield VNH2SP30 to control 2 DC motors.
 * - control DC motors by rotary encoders and PID
 * 
 * 03.02.18 wjtseng93
 */
//====================================================================================
// I2C
#include <Wire.h>
#define SLAVE_ADDRESS 0x12
#define SERIAL_BAUD 57600 
//====================================================================================
// Monster Motor Driver VNH2SP30
#define BRAKE 0
#define CW    1
#define CCW   2
#define CS_THRESHOLD 15   // Definition of safety current (Check: "1.3 Monster Shield Example").

//MOTOR 1
#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

//MOTOR 2
#define MOTOR_A2_PIN 4
#define MOTOR_B2_PIN 9

#define PWM_MOTOR_1 5
#define PWM_MOTOR_2 6

#define CURRENT_SEN_1 A2
#define CURRENT_SEN_2 A3

#define EN_PIN_1 A0
#define EN_PIN_2 A1

#define MOTOR_1 0
#define MOTOR_2 1

//short usSpeed = 50;  //default motor speed
short speedLeft = 255;
short speedRight = 255;
unsigned short usMotor_Status = BRAKE;

//====================================================================================
// Interrupt and Rotary encoders
// Arduino Leonardo has 4 interrupt Pin, D3, D2, D0, D1
// D0, D1, D2, and D3 for two rotary encoders
int encoderLeftPin1 = 3; // interrupt 0
int encoderLeftPin2 = 2; // interrupt 1
int encoderRightPin1 = 0;// interrupt 2
int encoderRightPin2 = 1;// interrupt 3

// rotary encoders
volatile long leftLastEncoded = 0;
volatile long rightLastEncoded = 0;

volatile long encoderLeftValue = 0;
volatile long encoderRightValue = 0;

//====================================================================================
// PID motor control
#include <PID_v1.h>
double kp = 0.4, ki = 0.06, kd = 0.01;
// input: current position (value of rotary encoder)
// output: result (where to go)
// setPoint: target position (position cmd from Feather)
double inputLeft = 0, outputLeft = 0, setPointLeft = 0;
double inputRight = 0, outputRight = 0, setPointRight = 0;
PID leftPID(&inputLeft, &outputLeft, &setPointLeft, kp, ki, kd, DIRECT); // DIRECT was defined in PID_v1.h
PID rightPID(&inputRight, &outputRight, &setPointRight, kp, ki, kd, DIRECT);

String inputString = "";
bool stringComplete = false;

void setup()                         
{
  // Debug
  pinMode(LED_BUILTIN, OUTPUT);
  
  // I2C setup
  Wire.begin(SLAVE_ADDRESS);    // join I2C bus as a slave with address 1
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(SERIAL_BAUD);

  // DC motor setup
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);

  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  pinMode(PWM_MOTOR_1, OUTPUT);
  pinMode(PWM_MOTOR_2, OUTPUT);

  pinMode(CURRENT_SEN_1, OUTPUT);
  pinMode(CURRENT_SEN_2, OUTPUT);  

  pinMode(EN_PIN_1, OUTPUT);
  pinMode(EN_PIN_2, OUTPUT);

 // rotary encoder setup
  pinMode(encoderLeftPin1, INPUT); 
  pinMode(encoderLeftPin2, INPUT);
  pinMode(encoderRightPin1, INPUT); 
  pinMode(encoderRightPin2, INPUT);

  digitalWrite(encoderLeftPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderLeftPin2, HIGH); //turn pullup resistor on
  digitalWrite(encoderRightPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderRightPin2, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  //on interrupt 2 (pin 0), or interrupt 3 (pin 1) 
  attachInterrupt(digitalPinToInterrupt(encoderLeftPin1), updateLeftEncoder, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(encoderLeftPin2), updateLeftEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderRightPin1), updateRightEncoder, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(encoderRightPin2), updateRightEncoder, CHANGE);

  // PID control setup
  leftPID.SetMode(AUTOMATIC);
  rightPID.SetMode(AUTOMATIC);
  leftPID.SetOutputLimits(-speedLeft, speedLeft);
  rightPID.SetOutputLimits(-speedRight, speedRight);

//  Serial.begin(9600);              // Initiates the serial to do the monitoring 
}

void loop() 
{
  inputLeft = encoderLeftValue;
  inputRight = encoderRightValue;

//  Serial.print(inputLeft); Serial.print(" ");
//  Serial.print(setPointLeft); Serial.print(" ");
//  Serial.print(outputLeft); Serial.print(" ");
//
//  Serial.print(inputRight); Serial.print(" ");
//  Serial.print(setPointRight); Serial.print(" ");
//  Serial.println(outputRight); Serial.print(" ");

  // control encoderLeftValue
  motorPIDControl(&encoderLeftValue, &setPointLeft, &outputLeft, &leftPID, EN_PIN_1, MOTOR_1);
  motorPIDControl(&encoderRightValue, &setPointRight, &outputRight, &rightPID, EN_PIN_2, MOTOR_2);

  // receive data from serial port
  while(Serial.available())
  {
//    Serial.println("get data");
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(EN_PIN_2, HIGH); 
    char c = Serial.read();
    inputString += c;
    if (c == '\n') {
      stringComplete = true;   
    }
  }
  if (stringComplete) {
    if (inputString.startsWith("L")) {
      inputString = inputString.substring(2);
      // split cmd into angle and speed
      for (int i = 0; i < inputString.length(); i++)
      {
        if (inputString.substring(i, i + 1) == " ")
        {  
          setPointLeft = (double) inputString.substring(0, i).toInt();
          speedLeft = (short) inputString.substring(i + 1).toInt();
          break;
        }
      }
      leftPID.SetOutputLimits(-speedLeft, speedLeft);
//      setPointLeft = (double) inputString.toInt();
//      stringComplete = false;
//      inputString = "";
    }
    else if (inputString.startsWith("R")) {
      inputString = inputString.substring(2);
      for (int i = 0; i < inputString.length(); i++)
      {
        if (inputString.substring(i, i + 1) == " ")
        {  
          setPointRight = (double) inputString.substring(0, i).toInt();
          speedRight = (short) inputString.substring(i + 1).toInt();
          break;
        }
      }
      rightPID.SetOutputLimits(-speedLeft, speedLeft);
//      setPointRight = (double) inputString.toInt();
//      stringComplete = false;
//      inputString = "";      
    }
    stringComplete = false;
    inputString = "";
  } 
}


// add I2C receive data code here 03.02
void receiveEvent(int count) {
  while (Wire.available()) {
    Serial.println("in wire.available");
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(EN_PIN_2, HIGH); 
    char c = (char) Wire.read();
    inputString += c;
    if (c == '\n') {
      stringComplete = true;   
    }
  }
  if (stringComplete) {
    Serial.println("in stringComplete");
    if (inputString.startsWith("L")) {
      inputString = inputString.substring(2);
      // split cmd into angle and speed
      for (int i = 0; i < inputString.length(); i++)
      {
        if (inputString.substring(i, i + 1) == " ")
        {  
          Serial.println("before assign value");
          Serial.print(setPointLeft); Serial.print(" ");
          Serial.println(speedLeft);
          setPointLeft = (double) inputString.substring(0, i).toInt();
          speedLeft = (short) inputString.substring(i + 1).toInt();
          Serial.println("after assign value");
          Serial.print(setPointLeft); Serial.print(" ");
          Serial.println(speedLeft);

          
          break;
        }
      }
      leftPID.SetOutputLimits(-speedLeft, speedLeft);
    }
    else if (inputString.startsWith("R")) {
      inputString = inputString.substring(2);
      for (int i = 0; i < inputString.length(); i++)
      {
        if (inputString.substring(i, i + 1) == " ")
        {  
          setPointRight = (double) inputString.substring(0, i).toInt();
          speedRight = (short) inputString.substring(i + 1).toInt();
          break;
        }
      }
      rightPID.SetOutputLimits(-speedLeft, speedLeft);
      
    }
    stringComplete = false;
    inputString = "";
    digitalWrite(LED_BUILTIN, LOW);
  }
} 

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)         //Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
{
  if(motor == MOTOR_1)
  {
    if(direct == CW)
    {
      digitalWrite(MOTOR_A1_PIN, LOW); 
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == CCW)
    {
      digitalWrite(MOTOR_A1_PIN, HIGH);
      digitalWrite(MOTOR_B1_PIN, LOW);      
    }
    else
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_1, pwm); 
  }
  else if(motor == MOTOR_2)
  {
    if(direct == CW)
    {
      // CW input for right motor is actually CCW
      digitalWrite(MOTOR_A2_PIN, HIGH);
      digitalWrite(MOTOR_B2_PIN, LOW);
    }
    else if(direct == CCW)
    {
      // CCW input for right motor is actually CW
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, HIGH);      
    }
    else
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_2, pwm);
  }
}

void updateLeftEncoder() {
  long MSB = digitalRead(encoderLeftPin1); //MSB = most significant bit
  long LSB = digitalRead(encoderLeftPin2); //LSB = least significant bit

  long encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  long sum  = (leftLastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderLeftValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderLeftValue --;

  leftLastEncoded = encoded; //store this value for next time
}

void updateRightEncoder() {
  long MSB = digitalRead(encoderRightPin1); //MSB = most significant bit
  long LSB = digitalRead(encoderRightPin2); //LSB = least significant bit

  long encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  long sum  = (rightLastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderRightValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderRightValue --;

  rightLastEncoded = encoded; //store this value for next time
}

void motorPIDControl(volatile long *encoderValue, double *setPoint, double *output, PID *motorPID, uint8_t EN_PIN, uint8_t MOTOR) {
//  if (*encoderValue > 500) {
//    *setPoint = 500;
//    motorPID->Compute();
//    motorGo(MOTOR, CCW, abs(*output));
//    if (*encoderValue == 500) {
//      motorGo(MOTOR, BRAKE, 0);
//      *output = 0;
//      digitalWrite(EN_PIN, LOW); 
//    }
//  } 
//  else if (*encoderValue < 0) {
//    setPoint = 0;
//    motorPID->Compute();

//    motorGo(MOTOR, CW, *output);
//    if (*encoderValue == 0) {
//      motorGo(MOTOR, BRAKE, 0);
//      *output = 0;
//      digitalWrite(EN_PIN, LOW);
//    }
//  }
//  else {
    motorPID->Compute();

    if (*output > 0) {
      motorGo(MOTOR, CW, *output);
    }
    else {
      motorGo(MOTOR, CCW, abs(*output));
    }
    if (*encoderValue == *setPoint) {
      motorGo(MOTOR, BRAKE, 0);
      *output = 0;
    }
//  }
}

// old version code
//  if (encoderLeftValue > 500) {
//    setPointLeft = 475;
//    leftPID.Compute();
//    motorGo(MOTOR_1, CCW, abs(outputLeft));
//    if (encoderLeftValue == 490) {
//      motorGo(MOTOR_1, BRAKE, 0);
//      outputLeft = 0;
//         digitalWrite(EN_PIN_1, LOW); 
//    }
//  } 
//  else if (encoderLeftValue < 0) {
//    setPointLeft = 10;
//    leftPID.Compute();
//
//    motorGo(MOTOR_1, CW, outputLeft);
//    if (encoderLeftValue == 10) {
//      motorGo(MOTOR_1, BRAKE, 0);
//      outputLeft = 0;
//      digitalWrite(EN_PIN_1, LOW);
//
//    }
//  }
//  else {
//    leftPID.Compute();
//
//    if (outputLeft > 0) {
//      motorGo(MOTOR_1, CW, outputLeft);
//    }
//    else {
//      motorGo(MOTOR_1, CCW, abs(outputLeft));
//    }
//    if (encoderLeftValue == setPointLeft) {
//      motorGo(MOTOR_1, BRAKE, 0);
//      outputLeft = 0;
//    }
//  }
