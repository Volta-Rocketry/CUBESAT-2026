#include <SoftwareSerial.h>

SoftwareSerial BT(10, 11); //TX AND RX

// Baud Rate
int BAUD_RATE = 9600;

// Time intervals
int SENSOR_READ_INTERVAL = 1;

// PIN definition
int PIN_VALVE_1 = 2;
int PIN_VALVE_2 = 3;
int PIN_TRANSDUCER = A0;

// Communication constants
String OPEN_VALVE_1 = "A1";
String CLOSE_VALVE_1 = "S1";
String OPEN_VALVE_2 = "A2";
String CLOSE_VALVE_2 = "S2";
String SEND_SIGNAL = "R";

// Variables
float lastSensorReadTime = 0;
String command;
int stateValve1;
int stateValve2;
// float transducerValue
// float voltage
// float pressureMpa

void setup() {
  Serial.begin(BAUD_RATE);
  BT.begin(38400);
  /*
  // Transducer initialized
  pinMode(PIN_TRANSDUCER, INPUT);
  Serial.println("Transducer initialized");
  */
  // Valves initialized
  pinMode(PIN_VALVE_1, OUTPUT);
  pinMode(PIN_VALVE_2, OUTPUT);
  Serial.println("Valves initialized and closed");
}

void loop() {
  float currentTime = millis()/1000;
  if (BT.available() >0 ){
    command = BT.readStringUntil("\n");
    command.trim();
    Serial.println(command);
    if (command == OPEN_VALVE_1) {
      digitalWrite(PIN_VALVE_1, HIGH);
      BT.println("Valve 1 opened");
      stateValve1 = 1;
    } 
    else if (command == CLOSE_VALVE_1) {
      digitalWrite(PIN_VALVE_1, LOW);
      BT.println("Valve 1 closed");
      stateValve1 = 0;
    } 
    else if (command == OPEN_VALVE_2) {
      digitalWrite(PIN_VALVE_2, HIGH);
      BT.println("Valve 2 opened");
      stateValve2 = 1;
    } 
    else if (command == CLOSE_VALVE_2) {
      digitalWrite(PIN_VALVE_2, LOW);
      BT.println("Valve 2 closed");
      stateValve2 = 0;
    } 
    else if (command == SEND_SIGNAL) {
      BT.print("M, I'm alive");
    } 
    else {
      BT.println("Unknown command received");
    }
  }
  if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentTime;
    /*
    transducerValue = analogRead(PIN_TRANSDUCER);
    voltage = transducerValue * (5.0 / 1023.0);
    pressureMpa = ((voltage - 0.5) / 4.0) * 0.5;
    if (pressureMpa < 0.0) {
      pressureMpa = 0.0;
    }
      */
    BT.print("D,");
    BT.print(stateValve1);
    BT.print(",");
    BT.println(stateValve2);
  }
}
