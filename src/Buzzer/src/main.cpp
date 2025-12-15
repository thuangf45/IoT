#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- 1. Cấu hình Wi-Fi ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- 2. Cấu hình MQTT ---
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* topic = "23clc06/23127491-23127483-23127467/out";

WiFiClient espClient;
PubSubClient client(espClient);

// --- 3. Cấu hình Chân Buzzer ---
const int buzzerPin = 12; // Nhớ nối chân dương vào GPIO 12

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);

  // Setup Buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); 

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// --- HÀM XỬ LÝ QUAN TRỌNG ĐÃ SỬA ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Tin nhắn nhận được: ");
  Serial.println(message);

  // Dùng ArduinoJson để giải mã gói tin {"type":"Notify"}
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Lỗi JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Lấy dữ liệu trường "type"
  const char* type = doc["type"]; 

  // Kiểm tra nếu type là "Notify" hoặc "Notify Buzzer" (tùy bạn đặt bên Node-RED)
  // Hàm strcmp dùng để so sánh 2 chuỗi, nếu bằng 0 là giống nhau
  if (type && strcmp(type, "Notify") == 0) { 
    Serial.println("!!! CẢNH BÁO: KÍCH HOẠT BUZZER !!!");
    
    // Kêu tít tít 3 lần
    for(int i=0; i<3; i++){
      digitalWrite(buzzerPin, HIGH); 
      delay(200);                    
      digitalWrite(buzzerPin, LOW);  
      delay(200);                    
    }
    Serial.println("--------");
  } else {
    Serial.println("-> Không phải lệnh Notify, bỏ qua.");
  }
}

// --- Các hàm kết nối giữ nguyên ---
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
  Serial.println("\nWiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-Buzzer-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic); 
    } else {
      delay(5000);
    }
  }
}