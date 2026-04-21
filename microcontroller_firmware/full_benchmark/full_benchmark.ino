#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <PubSubClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverIp = "YOUR_SERVER_IP";

const int httpPort = 3000;
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const int BATCH_SIZE = 100;

void setup_wifi() {
  Serial.print("\nConnecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP-FullBenchmark")) {
      Serial.println("connected");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(serverIp, mqttPort);
}

void loop() {
  Serial.println("\n=== Starting NEW BENCHMARK Cycle ===");
  
  Serial.printf("Running %d HTTP requests...\n", BATCH_SIZE);
  static float lastHttpDuration = 0;

  HTTPClient http;
  http.setReuse(true); 
  http.begin(wifiClient, "http://" + String(serverIp) + ":" + String(httpPort) + "/api/data/full");
  http.addHeader("Content-Type", "application/json");

  for (int i = 1; i <= BATCH_SIZE; i++) {
    unsigned long start = micros();

    StaticJsonDocument<256> doc;
    doc["test_id"] = "BENCHMARK_FULL";
    doc["device_ts"] = start;
    doc["msg_num"] = i;
    doc["sensor_val"] = 42.42;
    doc["device_active_ms"] = lastHttpDuration;
    
    String payload;
    serializeJson(doc, payload);
    
    int code = http.POST(payload);
    
    lastHttpDuration = (micros() - start) / 1000.0;
  }
  http.end();
  Serial.println("HTTP Batch Done.");

  delay(5000);

  reconnectMqtt();
  Serial.printf("Running %d MQTT publishes...\n", BATCH_SIZE);

  static float lastMqttDuration = 0;

  for (int i = 0; i < BATCH_SIZE; i++) {
    unsigned long start = micros();
    
    StaticJsonDocument<200> doc;
    doc["id"] = i;
    doc["device_active_ms"] = lastMqttDuration;
    
    String payload;
    serializeJson(doc, payload);
    
    mqttClient.publish("sensors/full", payload.c_str());
    mqttClient.loop();
    
    lastMqttDuration = (micros() - start) / 1000.0;
  }
  Serial.println("MQTT Batch Done.");

  Serial.println("\nCycles complete. Reseting in 30 seconds...");
  delay(30000);
}
