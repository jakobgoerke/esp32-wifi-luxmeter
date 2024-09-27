#include <SPI.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

Adafruit_TSL2591 tsl(2591);

static const char* ssid = ENV_WIFI_NAME;
static const char* password = ENV_WIFI_PASS;
static const char* hostname = "esp32-wifi-luxmeter";

void setupWifi() {
  WiFi.setHostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(ssid, password);

  Serial.print("\nESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP32 HostName: ");
  Serial.println(WiFi.getHostname());
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
}

void setupSensor() {
  if (!tsl.begin()) {
    Serial.println("Could not connect to sensor...");
    return;
  }

  Serial.println("Sensor connected...");
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
}

float getSensorData() {
  uint16_t reading = tsl.getLuminosity(TSL2591_FULLSPECTRUM);

  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;

  return tsl.calculateLux(full, ir);
}

void setupApi() {
  AsyncWebServer server(80);

  server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse * response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    root["lux"] = getSensorData();
    response->setLength();

    request->send(response);
  });

  server.begin();
}

void setup(){
  Serial.begin(115200);
  delay(1000);

  setupWifi();
  setupSensor();
  setupApi();
}

void loop(){}