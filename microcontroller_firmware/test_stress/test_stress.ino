#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <PubSubClient.h>

// --- WiFi & Server Config ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverIp = "YOUR_SERVER_IP";

const int httpPort = 3000;
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnectMqtt() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32-StressTester")) {
      Serial.println("MQTT Connected");
    } else {
      delay(2000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(serverIp, mqttPort);
}

bool testExecuted = false;

void loop() {
  if (testExecuted) {
    delay(1000);
    return;
  }

  Serial.println("\n!!! STARTING CLEAN STRESS TEST (1,000 messages per protocol) !!!");
  
  unsigned long httpStart = millis();
  HTTPClient http;
  http.setReuse(true); 
  String url = "http://" + String(serverIp) + ":" + String(httpPort) + "/api/data/stress";
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");

  for (int i = 1; i <= 1000; i++) {
    StaticJsonDocument<256> doc;
    doc["test_id"] = "STRESS_TEST_HTTP";
    doc["device_ts"] = micros();
    doc["payload"] = "STRESS TEST BURST DATA";
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    http.POST(jsonPayload);
  }
  http.end();

  unsigned long httpEnd = millis();
  Serial.printf("HTTP 1000 messages finished in %lu ms\n", httpEnd - httpStart);

  delay(5000);

  reconnectMqtt();
  unsigned long mqttStart = millis();
  for (int i = 1; i <= 1000; i++) {
    StaticJsonDocument<256> doc;
    doc["test_id"] = "STRESS_TEST_MQTT";
    doc["device_ts"] = micros();
    doc["payload"] = "STRESS TEST BURST DATA";
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    mqttClient.publish("sensors/stress", jsonPayload.c_str(), false);
  }
  unsigned long mqttEnd = millis();
  Serial.printf("MQTT 1000 messages finished in %lu ms\n", mqttEnd - mqttStart);

  Serial.println("\n***********************************");
  Serial.println("   CLEAN TEST COMPLETE!            ");
  Serial.println("***********************************");

  testExecuted = true;
}
