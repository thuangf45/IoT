#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h"

/* ================= WIFI ================= */
const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* ================= MQTT ================= */
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* topicPub = "23clc06/23127491-23127483-23127467/in";
const char* topicSub = "23clc06/23127491-23127483-23127467/out";

WiFiClient espClient;
PubSubClient client(espClient);

/* ================= PIN ================= */
#define LDR_PIN     32
#define LED_PIN     18
#define RELAY_PIN   17
#define BUZZER_PIN  14

#define TRIG_PIN    25
#define ECHO_PIN    26

#define ENC_CLK     16
#define ENC_DT      4

#define DHT_PIN     15

/* ================= THAM SỐ ================= */
#define DARK_THRESHOLD 1200
float wheelCircumference = 1.57;   // mét
int pulsesPerRevolution = 2;

/* ================= BIẾN ================= */
DHTesp dht;
volatile int pulseCount = 0;
unsigned long lastSend = 0;

/* ================= ENCODER ISR ================= */
void IRAM_ATTR readEncoder() {
  pulseCount++;
}

/* ================= WIFI ================= */
void setup_wifi() {
  Serial.println("WiFi connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}


/* ================= MQTT ================= */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX); 
    if (client.connect(clientId.c_str())) {
      client.subscribe(topicSub);
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
/* ================= CALLBACK ================= */
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (uint8_t i = 0; i < length; i++) msg += (char)payload[i];

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, msg)) return;

  const char* type = doc["type"];

  // Relay
  if (type && strcmp(type, "Lock") == 0) {
    digitalWrite(RELAY_PIN, LOW);
  }
  else if (type && strcmp(type, "Unlock") == 0) {
    digitalWrite(RELAY_PIN, HIGH);
  }

  // Buzzer
  else if (type && strcmp(type, "Notify") == 0) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);
    }
  }
}

/* ================= SENSOR ================= */
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2.0;
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), readEncoder, RISING);

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

/* ================= LOOP ================= */
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // LED theo ánh sáng
  int light = analogRead(LDR_PIN);
  digitalWrite(LED_PIN, (light < DARK_THRESHOLD));

  // gửi dữ liệu mỗi 2s
  if (millis() - lastSend >= 2000) {
    lastSend = millis();

    float distance = getDistance();
    TempAndHumidity dhtData = dht.getTempAndHumidity();

    float rev = (float)pulseCount / pulsesPerRevolution;
    float velocity = rev * wheelCircumference * 3.6;
    pulseCount = 0;

    StaticJsonDocument<256> doc;
    doc["brightness"]  = light;
    doc["distance"]    = distance;
    doc["temperature"] = dhtData.temperature;
    doc["humidity"]    = dhtData.humidity;
    doc["velocity"]    = velocity;
    doc["type"]        = "Data";

    char buffer[256];
    serializeJson(doc, buffer);
    client.publish(topicPub, buffer);

    Serial.println(buffer);
  }
}
