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
#define LDR_PIN 32    // AO của LDR (ADC1)
#define LED_PIN 2     // LED

/* ================= NGƯỠNG ================= */
#define DARK_THRESHOLD 1200   // Trời tối

/* ================= HÀM ================= */
void setup_wifi();
void reconnect();

/* ===== ĐỌC CƯỜNG ĐỘ ÁNH SÁNG ===== */
int getLightADC() {
  int value = analogRead(LDR_PIN);
  if (value < 0 || value > 4095) return -1;
  return value;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int lightValue = getLightADC();

  /* ===== LOGIC LED ===== */
  if (lightValue != -1 && lightValue < DARK_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  /* ===== GỬI MQTT ===== */
  StaticJsonDocument<200> doc;
  doc["brightness"] = lightValue;
  doc["type"] = "Data";
 

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