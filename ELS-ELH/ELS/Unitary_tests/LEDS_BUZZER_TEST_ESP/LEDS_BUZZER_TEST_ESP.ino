#define LED1 27  // R
#define LED2 14  // G
#define LED3 13  // B
#define BUZZER 17

void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

// ---------------- PWM LED (software) ----------------
void pwmFade(int pin) {

  // subida
  for (int duty = 0; duty <= 255; duty++) {
    for (int i = 0; i < 255; i++) {
      digitalWrite(pin, (i < duty) ? HIGH : LOW);
      delayMicroseconds(30);
    }
    delay(2);
  }

  // bajada
  for (int duty = 255; duty >= 0; duty--) {
    for (int i = 0; i < 255; i++) {
      digitalWrite(pin, (i < duty) ? HIGH : LOW);
      delayMicroseconds(30);
    }
    delay(2);
  }
}

// ---------------- LED colores ----------------
void setColor(bool r, bool g, bool b) {
  digitalWrite(LED1, r);
  digitalWrite(LED2, g);
  digitalWrite(LED3, b);
}

void blinkColor(const char* name, bool r, bool g, bool b) {
  Serial.print("Blinking ");
  Serial.println(name);

  for (int i = 0; i < 3; i++) {
    setColor(r, g, b);
    delay(300);
    setColor(0, 0, 0);
    delay(300);
  }
}

// ---------------- BUZZER ON/OFF ----------------
void beep(int onTime, int offTime, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(onTime);
    digitalWrite(BUZZER, LOW);
    delay(offTime);
  }
}

// ---------------- BUZZER PWM (software) ----------------
void playTone(int freq, int duration_ms) {
  int period_us = 1000000 / freq;
  int half_period = period_us / 2;

  long cycles = (long)freq * duration_ms / 1000;

  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(half_period);
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(half_period);
  }
}

void buzzerPWMTest() {
  Serial.println("Starting PWM buzzer test...");

  Serial.println("Low tone (500 Hz)");
  playTone(500, 1000);

  delay(300);

  Serial.println("Mid tone (1000 Hz)");
  playTone(1000, 1000);

  delay(300);

  Serial.println("High tone (2000 Hz)");
  playTone(2000, 1000);

  delay(300);

  Serial.println("Sweep test...");
  for (int f = 500; f <= 2000; f += 100) {
    playTone(f, 100);
  }

  delay(1000);
}

// ---------------- LOOP ----------------
void loop() {

  Serial.println("Starting test LED 1...");
  pwmFade(LED1);

  Serial.println("Starting test LED 2...");
  pwmFade(LED2);

  Serial.println("Starting test LED 3...");
  pwmFade(LED3);

  // Colores
  Serial.println("Starting color blink test...");
  blinkColor("Orange", 1, 1, 0);
  blinkColor("Magenta", 1, 0, 1);
  blinkColor("Cyan", 0, 1, 1);

  // Buzzer simple
  Serial.println("Starting buzzer ON/OFF test...");
  beep(100, 100, 5);

  delay(500);

  beep(300, 300, 3);

  delay(500);

  beep(50, 50, 10);

  // Buzzer PWM (frecuencia)
  buzzerPWMTest();

  delay(2000);
}