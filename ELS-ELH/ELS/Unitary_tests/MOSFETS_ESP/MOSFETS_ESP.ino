// ESP32 MOSFET TEST

#define mosfet 4
#define led 2

void setup(){
  pinMode(mosfet, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(mosfet, LOW);
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
}

void loop(){
  digitalWrite(mosfet, HIGH);
  digitalWrite(led, HIGH);
  delay(5000);

  digitalWrite(mosfet, LOW);
  digitalWrite(led, LOW);
  delay(10000);

  digitalWrite(mosfet, HIGH);
  digitalWrite(led, HIGH);
  delay(5000);

}