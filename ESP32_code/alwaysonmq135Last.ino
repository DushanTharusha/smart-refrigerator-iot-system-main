#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>
#include <atomic>

// WiFi credentials
const char* ssid = "Redmi note 9s";
const char* password = "binubinu";

// Azure Function URL
const char* functionUrl = "https://esp32csvlogger2.azurewebsites.net/api/uploadcsv";

// DHT sensor setup
#define DHT11_PIN 15
#define DHT22_PIN 4
DHT dht11(DHT11_PIN, DHT11);
DHT dht22(DHT22_PIN, DHT22);

// DHT power control
#define POWER_DHT11 25
#define POWER_DHT22 26

// MQ135 analog pins
#define MQ135_1_PIN 34
#define MQ135_2_PIN 35

// MQ135 GND switching via GPIO
#define MQ135_1_GND_SWITCH 32
#define MQ135_2_GND_SWITCH 33

// Door reed switch with DO pin
#define DOOR_PIN 13
volatile std::atomic<int> doorOpenCount(0);
volatile unsigned long lastDoorTrigger = 0;  // For debounce

// Status LED
#define STATUS_LED 2

// Timing
unsigned long lastLogTime = 0;
const unsigned long logInterval = 60000;  // 1 minute

// Interrupt handler with debounce
void IRAM_ATTR doorOpenedISR() {
  unsigned long now = millis();
  if (now - lastDoorTrigger > 100) {  // 100ms debounce
    doorOpenCount++;
    lastDoorTrigger = now;
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized!");
}

String getDate() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", timeinfo);
  return String(buf);
}

String getTimeStr() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buf[9];
  strftime(buf, sizeof(buf), "%H:%M:%S", timeinfo);
  return String(buf);
}

void sendToAzure(float t11, float h11, float t22, float h22, int doorCount, int gas1, int gas2, String date, String timeStr) {
  HTTPClient http;
  http.begin(functionUrl);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"date\":\"" + date + "\",";
  payload += "\"time\":\"" + timeStr + "\",";
  payload += "\"temperature_dht11\":" + String(t11, 2) + ",";
  payload += "\"humidity_dht11\":" + String(h11, 2) + ",";
  payload += "\"temperature_dht22\":" + String(t22, 2) + ",";
  payload += "\"humidity_dht22\":" + String(h22, 2) + ",";
  payload += "\"doorCount\":" + String(doorCount) + ",";
  payload += "\"gas1\":" + String(gas1) + ",";
  payload += "\"gas2\":" + String(gas2);
  payload += "}";

  Serial.println("[Azure] POST Payload:");
  Serial.println(payload);

  int code = http.POST(payload);
  String response = http.getString();

  Serial.print("Response Code: ");
  Serial.println(code);
  Serial.println("Response: " + response);
  http.end();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  digitalWrite(STATUS_LED, HIGH);
  delay(150);
  digitalWrite(STATUS_LED, LOW);
}

void setup() {
  Serial.begin(115200);
  dht11.begin();
  dht22.begin();

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  pinMode(DOOR_PIN, INPUT);  // For reed switch module
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), doorOpenedISR, RISING);  // Or FALLING based on your module

  pinMode(MQ135_1_PIN, INPUT);
  pinMode(MQ135_2_PIN, INPUT);

  pinMode(POWER_DHT11, OUTPUT);
  pinMode(POWER_DHT22, OUTPUT);
  digitalWrite(POWER_DHT11, LOW);
  digitalWrite(POWER_DHT22, LOW);

  // Keep MQ135 sensors always ON via GND switching
  pinMode(MQ135_1_GND_SWITCH, OUTPUT);
  pinMode(MQ135_2_GND_SWITCH, OUTPUT);
  digitalWrite(MQ135_1_GND_SWITCH, HIGH);  // Permanently ON
  digitalWrite(MQ135_2_GND_SWITCH, HIGH);  // Permanently ON

  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (millis() - lastLogTime >= logInterval) {
    lastLogTime = millis();

    // Power ON DHT sensors only during reading
    digitalWrite(POWER_DHT11, HIGH);
    digitalWrite(POWER_DHT22, HIGH);
    delay(1000);  // Sensor stabilization

    float t11 = dht11.readTemperature();
    float h11 = dht11.readHumidity();
    float t22 = dht22.readTemperature();
    float h22 = dht22.readHumidity();

    int gas1 = analogRead(MQ135_1_PIN);
    int gas2 = analogRead(MQ135_2_PIN);
    int countSnapshot = doorOpenCount.load();

    // Power OFF DHT sensors to save energy
    digitalWrite(POWER_DHT11, LOW);
    digitalWrite(POWER_DHT22, LOW);

    Serial.print("Gas Sensor 1: ");
    Serial.print(gas1);
    Serial.print(" | Gas Sensor 2: ");
    Serial.println(gas2);

    if (!isnan(t11) && !isnan(h11) && !isnan(t22) && !isnan(h22)) {
      sendToAzure(t11, h11, t22, h22, countSnapshot, gas1, gas2, getDate(), getTimeStr());
    } else {
      Serial.println("⚠️ Sensor read failed.");
    }

    // Reset door count for next minute
    doorOpenCount = 0;
  }
}
