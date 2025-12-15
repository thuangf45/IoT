#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "hvsinh23@clc.fitus.edu.vn"
#define AUTHOR_PASSWORD "ulylhrabqbkbbcyy"

#define RECIPIENT_EMAIL "hvsinh23@clc.fitus.edu.vn"

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
}

void sendViolationEmail(float speed) {

  SMTPSession smtp;
  smtp.callback(smtpCallback);

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "He thong giao thong thong minh";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "CANH BAO VI PHAM GIAO THONG";
  message.addRecipient("Nguoi dung", RECIPIENT_EMAIL);

String htmlMsg =
  String("<h2>🔔 THÔNG BÁO HỆ THỐNG</h2>") +
  String("<p>Phương tiện của bạn đã <b>vượt quá tốc độ cho phép</b>.</p>") +
  String("<ul>") +
  String("<li><b>Vận tốc ghi nhận:</b> ") + String(speed) + String(" km/h</li>") +
  String("<li><b>Ngưỡng cho phép:</b> 50 km/h</li>") +
  String("</ul>") +
  String("<p>Vui lòng tuân thủ luật giao thông.</p>");


  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";

  if (!smtp.connect(&session)) {
    Serial.println("Không kết nối được SMTP!");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Lỗi gửi mail!");
  } else {
    Serial.println("📧 GỬI EMAIL THÀNH CÔNG!");
  }
}


#define SPEED_LIMIT 50.0
#define BUTTON_PIN 4

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    float tocDoGiaLap = 85.5;

    Serial.print("Tốc độ đo được: ");
    Serial.println(tocDoGiaLap);

    if (tocDoGiaLap > SPEED_LIMIT) {
      sendViolationEmail(tocDoGiaLap);
    } else {
      Serial.println("Tốc độ an toàn, không gửi mail");
    }

    delay(5000);
  }
}
