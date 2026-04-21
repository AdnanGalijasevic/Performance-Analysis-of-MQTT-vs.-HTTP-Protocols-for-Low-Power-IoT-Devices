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

const int BATCH_SIZE = 100;

String generateLargeString(int size) {
  String str = "";
  str.reserve(size);
  for (int i = 0; i < size; i++) {
    str += "A";
  }
  return str;
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.print("\nConnecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  mqttClient.setServer(serverIp, mqttPort);
  mqttClient.setBufferSize(4096); 
}

void loop() {
  Serial.println("\n=== Starting LARGE PAYLOAD Benchmark ===");
  
  StaticJsonDocument<3000> doc;
  doc["test_id"] = "LARGE_PAYLOAD_TEST";
  doc["device_ts"] = micros();
  doc["sensor_data_block"] = generateLargeString(2000); 
  
  String payload;
  serializeJson(doc, payload);

  Serial.printf("Payload generated. Size: %d bytes\n", payload.length());

  // HTTP BATCH
  // ---------------------------------------------------------
  Serial.println("Running HTTP Test...");
  unsigned long httpStart = millis();
  
  HTTPClient http;
  http.setReuse(true); 
  http.begin(wifiClient, "http://" + String(serverIp) + ":" + String(httpPort) + "/api/data/large");
  http.addHeader("Content-Type", "application/json");

  for (int i = 0; i < BATCH_SIZE; i++) {
    http.POST(payload);
  }
  http.end();
  Serial.printf("HTTP 50 Large Messages Done in %lu ms.\n", millis() - httpStart);

  delay(5000);

  // ---------------------------------------------------------
  // MQTT BATCH
  // ---------------------------------------------------------
  Serial.println("Running MQTT Test...");
  while (!mqttClient.connected()) {
    mqttClient.connect("ESP-LargePayload");
    delay(100);
  }

  unsigned long mqttStart = millis();
  for (int i = 0; i < BATCH_SIZE; i++) {
    bool success = mqttClient.publish("sensors/large", payload.c_str());
    if (!success) Serial.println("MQTT Publish FAILED");
    mqttClient.loop();
  }
  Serial.printf("MQTT 50 Large Messages Done in %lu ms.\n", millis() - mqttStart);

  Serial.println("\nReseting in 30 seconds for next cycle...");
  delay(30000);
}
