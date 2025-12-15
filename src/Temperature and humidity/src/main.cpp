#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include "DHTesp.h"      

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;

const char* topic = "23clc06/23127491-23127483-23127467/in"; 


const int DHT_PIN = 15;
DHTesp dhtSensor;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastTime = 0;
const unsigned long interval = 2000; 

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo cảm biến
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  // Sử dụng millis() để không chặn chương trình (non-blocking)
  if (now - lastTime >= interval) {
    lastTime = now;

    // 1. Đọc dữ liệu từ cảm biến
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    // Kiểm tra xem đọc có lỗi không
    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // 2. Tạo gói tin JSON theo đúng chuẩn đồ án
    // Node-RED của bạn đang check: if (payload.type === "Data")
    StaticJsonDocument<200> doc;
    
    doc["temperature"] = data.temperature;
    doc["humidity"]    = data.humidity;
    doc["type"]        = "Data"; // Quan trọng: Để Node-RED nhận diện đúng luồng xử lý

    char buffer[200];
    serializeJson(doc, buffer);

    // 3. Gửi lên MQTT
    Serial.print("Publishing: ");
    Serial.println(buffer);
    Serial.println(topic);
    client.publish(topic, buffer);
    Serial.println("-----------");
  }
  delay(2000);
}