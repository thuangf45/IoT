#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* topic = "23clc06/23127491/in";

WiFiClient espClient;
PubSubClient client(espClient);

// Encoder pins
const int pinCLK = 2;   // nối chân CLK của encoder vào GPIO2
const int pinDT  = 4;   // nối chân DT vào GPIO4 (tùy chọn, để xác định chiều quay)

volatile int pulseCount = 0;
unsigned long lastTime = 0;
float wheelCircumference = 1.57; // mét (ví dụ bánh xe đường kính 0.5m)
int pulsesPerRevolution = 1;   // số xung cho 1 vòng encoder (KY-040 thường ~20 bước/vòng)

// Hàm ngắt: mỗi lần có xung từ CLK thì tăng biến đếm
void IRAM_ATTR readEncoder() {
  pulseCount++;
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      // chỉ publish, không cần subscribe
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);

  pinMode(pinCLK, INPUT_PULLUP);
  pinMode(pinDT, INPUT_PULLUP);

  // Bắt ngắt khi có xung từ CLK
  attachInterrupt(digitalPinToInterrupt(pinCLK), readEncoder, RISING);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastTime >= 1000) { // mỗi giây tính vận tốc
    // số vòng/giây = số xung / số xung mỗi vòng
    float revPerSec = (float)pulseCount / pulsesPerRevolution;
    float velocity_mps = revPerSec * wheelCircumference; // m/s
    float velocity_kmh = velocity_mps * 3.6;             // km/h
    pulseCount = 0;
    lastTime = now;

    Serial.print("Velocity: ");
    Serial.print(velocity_kmh);
    Serial.println(" km/h");

    // gửi lên MQTT
    StaticJsonDocument<100> doc;
    doc["velocity"] = velocity_kmh;
    doc["type"] = "Data";
    char buffer[100];
    serializeJson(doc, buffer);
    client.publish(topic, buffer);
  }
}
