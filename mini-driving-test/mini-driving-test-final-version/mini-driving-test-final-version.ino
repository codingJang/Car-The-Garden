// Importing required libraries
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

// Setting up Bluetooth serial communication on pins 8 and 9
SoftwareSerial bt_serial(8, 9);

// Defining pins for various functionalities and components
#define rst 0
#define ENA 5
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 7
#define ENB 6
#define ss 10
#define mosi 11
#define miso 12
#define sck 13

#define LS A2
#define FS A1
#define RS A0 // change pin number
#define lightPin A3
#define trig A4
#define ech A5

// Defining constants for direction and mode control
#define FW 0
#define BW 1
#define TR 2
#define TL 3
#define ST 4
#define RFW 5
#define LFW 6
#define BTR 7
#define BTL 8
#define RBW 9
#define LBW 10

#define WAIT_MODE 0
#define RFS_MODE 1
#define LFS_MODE 2
#define T_PARKING_MODE 3
#define EMERGENCY_MODE 4
#define READY_MODE 5

// Initialization and configuration variables
bool is_rfid_tagged = false;
bool flag = false;
int thr = 120;
int dir = 0;
bool isLeft = false;
bool isForward = false;
bool isRight = false;
float mult = 0.9;
int spd = 150;
int spdBoost = 255;
int spd1 = 70;
int spd2 = 255;
int delay0 = 70;
int delay1 = 150;
int delay2 = 450;
int delay3 = 1000;
int delay4 = 1300;
bool blind = false;
unsigned long timeout = 2000;
unsigned long delayTime = 700;
unsigned long blindStarted;
long lastForward = -1;
long lastRight = -1;
long lastLeft = -1;
long lastSensed = -1;
long duration, cm;
int lht;
int now = RFS_MODE;
int bkup = 10000;
int prev = 10000;

// Initializing RFID module with defined pins
MFRC522 mfrc522(ss, rst); 
MFRC522::MIFARE_Key key; 

// RFID read buffer
byte buffer[30];
byte block = 6;
byte size = 18;
byte data;

// Initializes Light Tracking Modules
void INIT_LT_MODULES()
{
  pinMode(LS, INPUT);
  pinMode(FS, INPUT);
  pinMode(RS, INPUT);
}

// Initializes Motor Driver pins
void INIT_MOTOR_DRIVER()
{
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

// Initializes and modifies Speed Variables
void INIT_SPEED()
{
  spd *= mult;
  spdBoost *= mult;
  spd1 *= mult;
  spd2 *= mult;
}

// Checks if left line sensor detects a line
bool LT_IS_LEFT()
{
  int val = analogRead(LS);
  //Serial.print("left: ");
  //Serial.println(val);
  return (val > thr) ? true : false;
}

// Checks if forward line sensor detects a line
bool LT_IS_FORWARD()
{
  int val = analogRead(FS);
  //Serial.print("forward: ");
  //Serial.println(val);
  return (val > thr) ? true : false;
}

// Checks if right line sensor detects a line
bool LT_IS_RIGHT()
{
  int val = analogRead(RS);
  //Serial.print("right: ");
  //Serial.println(val);
  return (val > thr) ? true : false;
}

// Senses and processes line data from all sensors
void SENSE_LINE()
{
  Serial.println("Line Sensed");
  isLeft = LT_IS_LEFT();
  isForward = LT_IS_FORWARD();
  isRight = LT_IS_RIGHT();
  if(isForward) lastForward = millis();
  if(isRight) lastRight = millis();
  if(isLeft) lastLeft = millis();
  long temp = lastRight > lastLeft ? lastRight : lastLeft;
  lastSensed = temp > lastForward ? temp : lastForward;
}

// Implements T-Parking maneuver logic
void T_PARKING()
{
  dir = BTL;
  CAR_UPDATE();
  do{
    SENSE_LINE();
  }while(!isForward);
  //CAR_UPDATE(TR);
  //delay(50);

  dir = ST;
  CAR_UPDATE();
  delay(500);
  
  dir = BW;
  CAR_UPDATE();
  do{
    SENSE_LINE();
  }while(!isLeft);

  dir = FW;
  CAR_UPDATE();
  delay(200);
  dir = ST;
  CAR_UPDATE();
  delay(500);
  
  dir = BTR;
  CAR_UPDATE();
  do{
    SENSE_LINE();
  }while(millis() - lastSensed < 350);

  dir = ST;
  CAR_UPDATE();
  delay(500);

  dir = BW;
  CAR_UPDATE();
  delay(500);
  /*
  do{
    if(isLeft) dir = BTR;
    else if(isRight) dir = BTL;
    CAR_UPDATE();
    delay(30);
    dir = BW;
    CAR_UPDATE();
    delay(100);
    SENSE_LINE();
  }while(!isLeft || !isRight);
  
  dir = ST;
  CAR_UPDATE();
  delay(500);
  
  dir = BW;
  CAR_UPDATE();
  delay(1000);
  */
}

// Implements Right-First Search logic
void RFS()
{
  SENSE_LINE();
  
  if(!blind && isRight && lastForward >= 0)
  {
    if(millis() - lastForward < delay0)
    {
      blind = true;
      blindStarted = millis();
      dir = TR;
    }
  }
  else if(!blind && isForward && lastRight >= 0)
  {
    if(millis() - lastRight < delay0)
    {
      blind = true;
      blindStarted = millis();
      dir = TR;
    }
  }
  
  if(!blind)
  {
    if(isLeft && !isForward && !isRight) dir = TL;
    else if(isForward && !isLeft && !isRight) dir = RFW;
    else if(isRight && !isForward && !isLeft) dir = TR;
    else if(isLeft && isForward && !isRight) dir = RFW;
    else if(!isLeft && isForward && isRight) dir = TR;
    else if(isLeft && isForward && isRight) dir = TR;
    else if(!isLeft && !isForward && !isRight)
    {
      long t = millis();
      if(dir == RFW && t - lastSensed > delay1 && t - lastSensed < delay2)
        dir = BTR;
      if(dir == TR && t - lastSensed > delay3 && t - lastSensed < delay4)
        dir = BTR;
      if(dir == TL && t - lastSensed > delay3 && t - lastSensed < delay4)
        dir = BTL;
    }
  }
  else
  {
    // Serial.println("blind");
    if(millis()-blindStarted > delayTime)
      blind = false;
  }
  //Serial.println(dir);
  //Serial.println();
}

// Implements Left-First Search logic
void LFS()
{
  SENSE_LINE();
  
  if(!blind && isLeft && lastForward >= 0)
  {
    if(millis() - lastForward < delay0)
    {
      blind = true;
      blindStarted = millis();
      dir = TL;
    }
  }
  else if(!blind && isForward && lastLeft >= 0)
  {
    if(millis() - lastLeft < delay0)
    {
      blind = true;
      blindStarted = millis();
      dir = TL;
    }
  }
  
  if(!blind)
  {
    if(isRight && !isForward && !isLeft) dir = TR;
    else if(isForward && !isRight && !isLeft) dir = LFW;
    else if(isLeft && !isForward && !isRight) dir = TL;
    else if(isRight && isForward && !isLeft) dir = LFW;
    else if(!isRight && isForward && isLeft) dir = TL;
    else if(isRight && isForward && isLeft) dir = TL;
    else if(!isRight && !isForward && !isLeft)
    {
      long t = millis();
      if(dir == LFW && t - lastSensed > delay1 && t - lastSensed < delay2)
        dir = BTL;
      if(dir == TL && t - lastSensed > delay3 && t - lastSensed < delay4)
        dir = BTL;
      if(dir == TR && t - lastSensed > delay3 && t - lastSensed < delay4)
        dir = BTR;
    }
  }
  else
  {
    // Serial.println("blind");
    if(millis()-blindStarted > delayTime)
      blind = false;
  }
  //Serial.println(dir);
  //Serial.println();
}

// Updates the car's movement based on direction
void CAR_UPDATE()
{
  //Serial.print("CAR UPDATE: ");
  //Serial.println(dir);
  if(dir == FW)
  {
    //Serial.println("Forward");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spd);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spd);
  }
  else if(dir == BW)
  {
    //Serial.println("Backward");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, spd);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, spd);
  }
  else if(dir == TR)
  {
    //Serial.println("Turn_Right");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, spd);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spd);
  }
  else if(dir == TL)
  {
    //Serial.println("Turn_Left");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spd);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, spd);
  }
  else if(dir == ST)
  {
    //Serial.println("STOP");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spd);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spd);
  }
  if(dir == RFW)
  {
    //Serial.println("Forward");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spd1);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spd2);
  }
  if(dir == LFW)
  {
    //Serial.println("Forward");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spd2);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spd1);
  }
  else if(dir == BTR)
  {
    //Serial.println("Turn_Right");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, spdBoost);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, spdBoost);
  }
  else if(dir == BTL)
  {
    //Serial.println("Turn_Right");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, spdBoost);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, spdBoost);
  }
  else if(dir == RBW)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, spd1);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, spd2);
  }
  else if(dir == LBW)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, spd2);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, spd1);
  }
}

// Converts microseconds (from ultrasonic sensor) to distance in centimeters
long microsecondsToCentimeters(long microseconds){
  return  microseconds / 29 / 2;
}

// Senses the surroundings using ultrasonic and light sensors, and Bluetooth data
void SENSE_SURROUNDINGS(){
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(ech, HIGH, timeout);
  cm = microsecondsToCentimeters(duration!=0?duration:timeout);

  lht = digitalRead(lightPin);
  
  if (bt_serial.available() > 0){
    data = (byte) bt_serial.read();
  }
  else data = 0;
}

// Reads and processes RFID tag data
void SENSE_RFID(){
  mfrc522.PCD_Init();
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    is_rfid_tagged = true;
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key,&(mfrc522.uid));
    status = mfrc522.MIFARE_Read(block, buffer, &size);
  }
  else
  {
    is_rfid_tagged = false;
  }
}

// Changes the operational mode of the robot based on sensors and conditions
void CHANGE_MODE()
{
  if(cm < 20 || lht == 1){ // when the obstacle is detected
    // Serial.println(cm);
    if(now != WAIT_MODE && now != READY_MODE) // if it's not already in WAIT MODE
    {
      Serial.println("entered WAIT mode");
      bkup = now;
      prev = dir; // backup now and dir
      now = WAIT_MODE; // change now variable
    }
    SENSE_RFID();
    if(is_rfid_tagged) // when rfid is tagged
    {
      if(buffer[0] == 0x21){ // change mode accordingly
        byte mode = buffer[1];
        if(mode == 0x00){
          now = READY_MODE;
          bkup = RFS_MODE;
        }
        else if(mode == 0x01){
          now = READY_MODE;
          bkup = LFS_MODE;
        }
        else if(mode == 0x02){
          now = READY_MODE;
          bkup = T_PARKING_MODE;
        }
      }
    }
  }
  else if(now == WAIT_MODE || now == READY_MODE) // when obstacle is not detected and is in WAIT/READY
  {
    Serial.println("escaped WAIT/READY mode");
    now = bkup;
    dir = prev; // restore previous mode and direction
  }
  
  if(data == 'E'){ // when emergency signal is detected
    bkup = now;
    prev = dir; // backup now and dir
    bt_serial.print("EMERGENCY"); // print emergency
    now = EMERGENCY_MODE; // change now variable
  }
  else if(now == EMERGENCY_MODE) // when emergency signal is not detected and is in emergency mode
  {
    now = bkup;
    dir = prev; // restore now and dir
    // Serial.println("EMERGENCY ENDED");
  }

}

// Function to execute the actions associated with the current operational mode
void EXECUTE_MODE(){
  if(now == WAIT_MODE || now == READY_MODE){ // if in WAIT/READY mode
    dir = ST; // stop
  }
  else if(now == RFS_MODE){ // if in RFS mode
    RFS(); // execute RFS
  }
  else if(now == LFS_MODE){ // if in LFS mode
    LFS(); // execute RFS
  }
  else if(now == T_PARKING_MODE){ // if in T-parking mode
    T_PARKING(); // execute T-parking
    now = RFS_MODE; // return to RFS mode
  }
  else if(now == EMERGENCY_MODE){ // if in emergency mode
    dir = ST; // stop
    CAR_UPDATE(); // update car
    delay(3000); // delay for 3 seconds
  }
}

// Initial setup function that runs once on startup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  
  bt_serial.begin(9600);
  for(int i = 0; i<6;i++){   
    key.keyByte[i] = 0xff;
  }
  INIT_LT_MODULES();
  INIT_MOTOR_DRIVER();
  INIT_SPEED();
  pinMode(trig, OUTPUT);
  pinMode(ech, INPUT);
}


void loop() {
  SENSE_SURROUNDINGS();
  CHANGE_MODE();
  EXECUTE_MODE();
  CAR_UPDATE();
//   Serial.print(cm);
//   Serial.print("&&");
//   Serial.print(lht);
//   Serial.print("&&");
   Serial.println(now);
}
