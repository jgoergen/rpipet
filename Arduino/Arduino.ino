#include <Servo.h>

ADC_MODE(ADC_VCC);

// PIN DEFINITIONS //////////////////////////////////////////////////////////////////////

#define TURN_MOTOR_SPEED_PIN    5
#define TURN_MOTOR_DIR_PIN      0
#define DRIVE_MOTOR_SPEED_PIN   4
#define DRIVE_MOTOR_DIR_PIN     2
#define ECHO_PIN                12
#define TRIGGER_PIN             13
#define CAM_SERVO_PIN           15
#define BATTERY_VOLTAGE_PIN     A0

// SETTINGS /////////////////////////////////////////////////////////////////////////////

#define MOTORS_UPDATE_SPEED           100
#define SYSTEM_CHECK_SPEED            1000
#define CAM_SERVO_UPDATE_SPEED        33
#define MIN_ULTRASONIC_RANGE          10
#define MIN_BATTERY_VOLTAGE           4

// VARIABLES ////////////////////////////////////////////////////////////////////////////

const char COMMAND_BEGIN_CHAR = '<';
const char COMMAND_END_CHAR = '>';
const char* COMMAND_DIVIDER_CHAR = ":";
const char* COMMAND_DIVIDER_CHAR_SECONDARY = ",";
const byte COMMAND_LENGTH = 200;
int lastMotorUpdate = 0;
int lastCamServoUpdate = 0;
int lastSystemCheckUpdate = 0;
char receivedChars[COMMAND_LENGTH];
boolean newData = false;
char receivedNewData[COMMAND_LENGTH];
int movementDirection = 0;
int movementSteps = 0;
int camServoPosition = 0;
int camServoTargetPosition = 0;
int tooCloseCount = 0;

Servo camTilt;

// MAIN FUNCTIONS ///////////////////////////////////////////////////////////////////////

void setup() {
  
  Serial.begin(115200);

  pinMode(TURN_MOTOR_SPEED_PIN, OUTPUT);
  pinMode(TURN_MOTOR_DIR_PIN, OUTPUT);
  pinMode(DRIVE_MOTOR_SPEED_PIN, OUTPUT);
  pinMode(DRIVE_MOTOR_DIR_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  camServoPosition = camServoTargetPosition = 90;
  camTilt.attach(CAM_SERVO_PIN);
  camTilt.write(camServoPosition);
  
  randomSeed(1);
  Serial.print("ready|");
  Serial.println(readBatteryVoltage());
}

void loop() {

  checkSerialData();
  long milliseconds = millis();
 
  if (milliseconds - lastSystemCheckUpdate > SYSTEM_CHECK_SPEED) {
    
    lastSystemCheckUpdate = milliseconds;
    
    if (readBatteryVoltage() < MIN_BATTERY_VOLTAGE) {

      Serial.println("warning|battery"); 
    }
  }

  if (milliseconds - lastCamServoUpdate > CAM_SERVO_UPDATE_SPEED) {
    
    lastCamServoUpdate = milliseconds;

    if (camServoTargetPosition < camServoPosition) {

      camServoPosition -= floor((camServoPosition - camServoTargetPosition) / 5.0f);
      camTilt.write(camServoPosition);
    
    } else if (camServoTargetPosition > camServoPosition) {
      
      camServoPosition += floor((camServoTargetPosition - camServoPosition) / 5.0f);
      camTilt.write(camServoPosition);
    }
  }

  if (milliseconds - lastMotorUpdate > MOTORS_UPDATE_SPEED) {
    
    lastMotorUpdate = milliseconds;

    if (readUltrasonicSensorCM() < MIN_ULTRASONIC_RANGE) {

      tooCloseCount ++;

      if (tooCloseCount > 4) {
        
        Serial.println("warning|close");
        movementDirection = 0;
      }
    } else {

      tooCloseCount = 0;
    }
      
    if (movementDirection == 1 ) {

      // Center steering
      digitalWrite(TURN_MOTOR_DIR_PIN, LOW);
      digitalWrite(TURN_MOTOR_SPEED_PIN, LOW);
      // Strait Forward
      digitalWrite(DRIVE_MOTOR_DIR_PIN, HIGH);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 90;
    
    } else if(movementDirection == 2) {

      // Center steering
      digitalWrite(TURN_MOTOR_DIR_PIN, LOW);
      digitalWrite(TURN_MOTOR_SPEED_PIN, LOW);
      // Strait Backwards
      digitalWrite(DRIVE_MOTOR_DIR_PIN, LOW);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 90;
          
    } else if(movementDirection == 3) {

      // Right
      digitalWrite(TURN_MOTOR_DIR_PIN, HIGH);
      digitalWrite(TURN_MOTOR_SPEED_PIN, HIGH);
      // Forward
      digitalWrite(DRIVE_MOTOR_DIR_PIN, HIGH);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 50;
      
    } else if(movementDirection == 4) {

      // Left
      digitalWrite(TURN_MOTOR_DIR_PIN, LOW);
      digitalWrite(TURN_MOTOR_SPEED_PIN, HIGH);
      // Forward
      digitalWrite(DRIVE_MOTOR_DIR_PIN, HIGH);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 140;
    
    } else if(movementDirection == 5) {

      // Right
      digitalWrite(TURN_MOTOR_DIR_PIN, HIGH);
      digitalWrite(TURN_MOTOR_SPEED_PIN, HIGH);
      // Backwards
      digitalWrite(DRIVE_MOTOR_DIR_PIN, LOW);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 90;

    } else if(movementDirection == 6) {

      // Left
      digitalWrite(TURN_MOTOR_DIR_PIN, LOW);
      digitalWrite(TURN_MOTOR_SPEED_PIN, HIGH);
      // Backwards
      digitalWrite(DRIVE_MOTOR_DIR_PIN, LOW);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, HIGH);
      camServoTargetPosition = 90;
    
    } else {

      // Stop
      digitalWrite(TURN_MOTOR_SPEED_PIN, LOW);
      digitalWrite(DRIVE_MOTOR_SPEED_PIN, LOW);
      camServoTargetPosition = 90;
      // Center steering
      digitalWrite(TURN_MOTOR_DIR_PIN, LOW);
      digitalWrite(TURN_MOTOR_SPEED_PIN, LOW);
    }
  }
}

void processSerialCommand(char* command, char* param) {

  // commands should be in the format "<COMMAND:PARAMER>"

  if (strcmp(command, "getdist") == 0) {

    // read the ultrasonic range finder

    int dist = 9999;
    int measure = 9999;
    
    for (int i = 0; i < 4; i++) {

      measure = readUltrasonicSensorCM();
      
      if (measure < dist)
        dist = measure;
    }
    
    Serial.print("ok|");
    Serial.println(dist);
  
  } else if (strcmp(command, "campos") == 0) {

    // 0 - 180
    // change the position of the camera
    camServoTargetPosition = atoi(param);
    Serial.print("ok|");
    Serial.println(camServoTargetPosition);
    
  } else if (strcmp(command, "dir") == 0) {

    // 0 == stop
    // 1 == forward
    // 2 == backward
    // 3 == stop
    // 4 == right
    // 5 == left
    // 6 == center
    // change the vehicle movement
    movementDirection = atoi(param);
    Serial.print("ok|");
    Serial.println(movementDirection);

  } else if (strcmp(command, "bat") == 0) {

    Serial.print("ok|");
    Serial.println(readBatteryVoltage());
  }
}

void checkSerialData() {
  
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;
  
  while (Serial.available() > 0 && newData == false) {
    
    rc = Serial.read();

    if (recvInProgress == true) {
      
        if (rc != COMMAND_END_CHAR) {
          
            receivedChars[ndx] = rc;
            ndx++;
            
            if (ndx >= COMMAND_LENGTH)              
                ndx = COMMAND_LENGTH - 1;
        } else {
          
            receivedChars[ndx] = '\0'; // terminate the string
            recvInProgress = false;
            ndx = 0;
            newData = true;
        }
    }

    else if (rc == COMMAND_BEGIN_CHAR)      
        recvInProgress = true;
  }

  if (newData == true) {

    newData = false;
    memcpy(
      receivedNewData, 
      receivedChars, 
      COMMAND_LENGTH * sizeof(char));
      
  } else {

    receivedNewData[0] = 0;
    receivedNewData[1] = 0;
    receivedNewData[2] = 0;
  }

  if (receivedNewData[0] != 0 && receivedNewData[1] != 0 && receivedNewData[2] != 0) {
  
    // seperate the command from the parameter and process it
    
    char* command = strtok(receivedNewData, COMMAND_DIVIDER_CHAR);
    char* param = strtok(NULL, COMMAND_DIVIDER_CHAR);
    processSerialCommand(command, param);
  }
}

long readUltrasonicSensorCM() {
  
   long duration;
   
   digitalWrite(TRIGGER_PIN, LOW);
   delayMicroseconds(2);
   digitalWrite(TRIGGER_PIN, HIGH);
   delayMicroseconds(10);
   digitalWrite(TRIGGER_PIN, LOW);
   pinMode(ECHO_PIN, INPUT);
   duration = pulseIn(ECHO_PIN, HIGH);
   
   return duration / 29 / 2;
}

uint32_t readBatteryVoltage() {

  return ESP.getVcc();
}
