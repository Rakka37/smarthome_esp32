// ============================================================
// RAKKA LEVINE NATHANIEL
// ESP32 + Blynk + Relay + Sensor Suara + DHT11
// ============================================================

#define BLYNK_TEMPLATE_ID "TMPL617I3l73U"
#define BLYNK_TEMPLATE_NAME "raka"
#define BLYNK_AUTH_TOKEN "J0rc_iSpoBuT8aOMrQULsfpaxI1l-7ET"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// ====== WiFi ======
char ssid[] = "HUAWEI-2.4G-bUU5";
char pass[] = "indonesia";

// ====== PIN RELAY (AMAN ESP32) ======
#define RELAY_1 18
#define RELAY_2 19
#define RELAY_3 21
#define RELAY_4 22
#define RELAY_5 23
#define RELAY_6 13
#define RELAY_7 14

// ====== SENSOR SUARA ======
#define SENSOR_SUARA 34   // input only

// ====== DHT11 ======
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ====== Variabel ======
bool relay2State = false;
int prevValue = HIGH;
unsigned long lastTrigger = 0;
const unsigned long debounceDelay = 1000;

// Reconnect
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 10000;

// Timer
BlynkTimer timer;

// ====== Kirim Data DHT ======
void sendSensorDHT() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca DHT11");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C | Kelembapan: ");
  Serial.println(kelembapan);

  Blynk.virtualWrite(V9, suhu);
  Blynk.virtualWrite(V10, kelembapan);
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);
  pinMode(RELAY_7, OUTPUT);

  pinMode(SENSOR_SUARA, INPUT);

  // Relay OFF (aktif LOW)
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  digitalWrite(RELAY_5, LOW);
  digitalWrite(RELAY_6, LOW);
  digitalWrite(RELAY_7, LOW);

  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  Blynk.connect();
  dht.begin();

  timer.setInterval(5000L, sendSensorDHT);
}

// ============================================================
// BLYNK CONTROL
// ============================================================
BLYNK_WRITE(V0) { digitalWrite(RELAY_1, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V1) {
  relay2State = param.asInt();
  digitalWrite(RELAY_2, relay2State ? LOW : HIGH);
}
BLYNK_WRITE(V2) { digitalWrite(RELAY_3, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V3) { digitalWrite(RELAY_4, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V4) { digitalWrite(RELAY_5, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V5) { digitalWrite(RELAY_6, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V6) { digitalWrite(RELAY_7, param.asInt() ? LOW : HIGH); }

// ============================================================
// LOOP
// ============================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval) {
      Serial.println("WiFi reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, pass);
      lastReconnectAttempt = now;
    }
  }
  else if (!Blynk.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval) {
      Serial.println("Blynk reconnect...");
      Blynk.connect();
      lastReconnectAttempt = now;
    }
  }
  else {
    Blynk.run();
    timer.run();
  }

  // Sensor Suara → Relay 2
  int currentValue = digitalRead(SENSOR_SUARA);
  if (prevValue == HIGH && currentValue == LOW) {
    unsigned long now = millis();
    if (now - lastTrigger > debounceDelay) {
      relay2State = !relay2State;
      digitalWrite(RELAY_2, relay2State ? LOW : HIGH);
      Blynk.virtualWrite(V1, relay2State ? 1 : 0);

      Serial.println(relay2State ? "Relay 2 ON (Suara)" : "Relay 2 OFF (Suara)");
      lastTrigger = now;
    }
  }
  prevValue = currentValue;
}
