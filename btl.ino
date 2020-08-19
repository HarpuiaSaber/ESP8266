#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DHT.h"

#define FAN 12

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht (DHTPIN, DHTTYPE);

float t, h;
char message[9];

const char* ssid = "AndroidAP9450";
const char* password = "nmcv2543";

byte degree[8] = {
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);

  pinMode(FAN, OUTPUT);

  dht.begin();

  lcd.createChar(1, degree);
  lcd.init();  // khởi tạo màn hình
  lcd.backlight();  // bật đèn nền LCD

  startWiFi();
  connectWebSocket();
}

void loop() {
  hienthi_lcd();
  sprintf(message, "%g,%g", t, h);
  webSocket.sendTXT(message);               
  webSocket.loop();
  delay(2000);
}

void startWiFi() {
  WiFi.begin(ssid, password);           // Kết nối vào mạng WiFi
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Chờ kết nối WiFi được thiết lập
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n");
  Serial.println("Connection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());       // Gởi địa chỉ IP đến máy tinh
}

void connectWebSocket() {
  webSocket.begin("192.168.43.165", 8080, "/WebsocketServer/server");          // Địa chỉ websocket server, port và URL
  webSocket.onEvent(webSocketEvent);
  // webSocket.setAuthorization("user", "password");        // Sử dụng thông tin chứng thực nếu cần
  webSocket.setReconnectInterval(3000);                     // Thử lại sau 3s nếu kết nối không thành công
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:                         // Sự kiện khi client ngắt kết nối
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:                            // Sự kiện khi client kết nối
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      webSocket.sendTXT("ESP8266");          // Thông báo kết nối thành công
      break;
    case WStype_TEXT:                                 // Sự kiện khi nhận được thông điệp dạng TEXT
      Serial.printf("[WSc] get text: %s\n", payload);
      if (*payload == (char)49) {
        digitalWrite(FAN, HIGH);
      } else if (*payload == (char)48){
        digitalWrite(FAN, LOW);
      }
      break;
    case WStype_BIN:                                  // Sự kiện khi nhận được thông điệp dạng BINARY
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(message, length);
      // webSocket.sendBIN(payload, length);
      break;
  }
}

void hienthi_lcd() {
  t = dht.readTemperature();
  h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp1: ");// In
    lcd.setCursor(0, 1); // chuyển con trỏ tới hàng 1 cột 0
    lcd.print("Humi1: "); // In
    lcd.createChar(1, degree);
    lcd.setCursor(7, 0);
    lcd.print(t);
    lcd.print(" ");
    lcd.write(1); // viết ký tự độ
    lcd.print("C");
    lcd.setCursor(7, 1);
    lcd.print(h);
    lcd.print(" %");
  }
}
