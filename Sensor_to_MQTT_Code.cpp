{\rtf1\ansi\ansicpg1252\cocoartf2709
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\paperw11900\paperh16840\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 #include <WiFi.h>\
#include <PubSubClient.h>\
#include <Wire.h>\
#include <Adafruit_MPU6050.h>\
#include <Adafruit_Sensor.h>\
#include <DHT.h>\
#include <OneWire.h>\
#include <DallasTemperature.h>\
\
// ==================== WiFi and MQTT Configuration ====================\
const char* ssid = "";\
const char* password = "";\
const char* mqtt_server = ""; // Replace with your laptop IP (from ipconfig or Serial output)\
\
WiFiClient espClient;\
PubSubClient client(espClient);\
\
// ==================== Pin Configuration ====================\
#define FLOAT_SWITCH_PIN 12\
#define DHT22_PIN 25\
#define ACS712_PIN 34\
#define SDA_PIN 32\
#define SCL_PIN 33\
#define DS18B20_PIN 14\
\
// ==================== Sensor Instances ====================\
DHT dht22(DHT22_PIN, DHT22);\
Adafruit_MPU6050 mpu;\
OneWire oneWire(DS18B20_PIN);\
DallasTemperature ds18b20(&oneWire);\
\
// ==================== Global Variables ====================\
float vibrationThreshold = 0;\
const float ACS712_SENSITIVITY = 0.185; \
const float ACS712_OFFSET = 1.65;\
\
unsigned long lastPrint = 0;\
unsigned long printInterval = 2000;\
\
// ==================== Setup ====================\
void setup() \{\
  Serial.begin(115200);\
  delay(1000);\
\
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);\
  dht22.begin();\
  ds18b20.begin();\
\
  Wire.begin(SDA_PIN, SCL_PIN);\
  if (!mpu.begin()) \{\
    Serial.println("MPU6050 not detected!");\
    while (1) delay(10);\
  \}\
  Serial.println("MPU6050 initialized.");\
  calibrateVibrationThreshold();\
\
  analogReadResolution(12);\
\
  // WiFi setup\
  WiFi.begin(ssid, password);\
  Serial.print("Connecting to WiFi");\
  while (WiFi.status() != WL_CONNECTED) \{\
    delay(500);\
    Serial.print(".");\
  \}\
  Serial.println("\\nWiFi connected");\
  Serial.print("ESP32 IP: ");\
  Serial.println(WiFi.localIP());\
\
  // MQTT setup\
  client.setServer(mqtt_server, 1883);\
\}\
\
// ==================== Main Loop ====================\
void loop() \{\
  if (!client.connected()) \{\
    reconnectMQTT();\
  \}\
  client.loop();\
\
  unsigned long currentMillis = millis();\
  if (currentMillis - lastPrint >= printInterval) \{\
    lastPrint = currentMillis;\
\
    // Float Switch\
    int floatState = digitalRead(FLOAT_SWITCH_PIN);\
    client.publish("sensors/float", floatState == LOW ? "Water Detected" : "No Water");\
\
    // Vibration\
    float vibration = getVibrationMagnitude();\
    String vibrationStatus = (vibration > vibrationThreshold) ? "Abnormal" : "Normal";\
    client.publish("sensors/vibration", String(vibration).c_str());\
    client.publish("sensors/vibration_status", vibrationStatus.c_str());\
\
    // Current\
    int rawValue = analogRead(ACS712_PIN);\
    float voltage = (rawValue / 4095.0) * 3.3;\
    float current = (voltage - ACS712_OFFSET) / ACS712_SENSITIVITY;\
    client.publish("sensors/current", String(current).c_str());\
\
    // DHT22\
    float humi = dht22.readHumidity();\
    float tempC = dht22.readTemperature();\
    if (!isnan(humi)) \{\
      client.publish("sensors/humidity", String(humi).c_str());\
    \}\
    if (!isnan(tempC)) \{\
      client.publish("sensors/temp_dht22", String(tempC).c_str());\
    \}\
\
    // DS18B20\
    ds18b20.requestTemperatures();\
    float dsTempC = ds18b20.getTempCByIndex(0);\
    client.publish("sensors/temp_ds18b20", String(dsTempC).c_str());\
  \}\
\}\
\
// ==================== Helper Functions ====================\
float getVibrationMagnitude() \{\
  sensors_event_t a, g, temp;\
  mpu.getEvent(&a, &g, &temp);\
  return sqrt(a.acceleration.x * a.acceleration.x +\
              a.acceleration.y * a.acceleration.y +\
              a.acceleration.z * a.acceleration.z);\
\}\
\
void calibrateVibrationThreshold() \{\
  const int samples = 100;\
  float sum = 0, sumSq = 0;\
  for (int i = 0; i < samples; i++) \{\
    float mag = getVibrationMagnitude();\
    sum += mag;\
    sumSq += mag * mag;\
    delay(50);\
  \}\
  float mean = sum / samples;\
  float stddev = sqrt(sumSq / samples - mean * mean);\
  vibrationThreshold = mean + 2 * stddev;\
\}\
\
void reconnectMQTT() \{\
  while (!client.connected()) \{\
    Serial.print("Connecting to MQTT...");\
    if (client.connect("ESP32Client")) \{\
      Serial.println("connected.");\
    \} else \{\
      Serial.print("failed, rc=");\
      Serial.print(client.state());\
      Serial.println(" trying again in 5s");\
      delay(5000);\
    \}\
  \}\
\}}
