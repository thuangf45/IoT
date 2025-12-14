#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ================= WIFI ================= */
const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* ================= MQTT ================= */
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* topic = "23clc06/23127491-23127483-23127467/in";

WiFiClient espClient;
PubSubClient client(espClient);

/* ================= CHÂN ================= */
const int trig_pin  = 25;
const int echo_pin  = 26;

/* ================= HÀM ================= */
void setup_wifi();
void reconnect();

/* ===== ĐO KHOẢNG CÁCH ===== */
float getDistance() {
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(10);

  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  long duration = pulseIn(echo_pin, HIGH, 30000);
  if (duration == 0) return -1;

  float distance = duration * 0.034 / 2.0;
  if (distance <= 0 || distance > 400) return -1;

  return distance;
}

void setup() {
  Serial.begin(115200);

  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float distance = getDistance();
  // Gửi MQTT
  StaticJsonDocument<200> doc;
  doc["distance"] = distance;
  doc["type"]= "Data";

  char buffer[128];
  serializeJson(doc, buffer);
  client.publish(topic, buffer);

  Serial.println(buffer);
  delay(2000);
}

/* ===== WIFI ===== */
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

/* ===== MQTT ===== */
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      // connected
    } else {
      delay(5000);
    }
  }
}
