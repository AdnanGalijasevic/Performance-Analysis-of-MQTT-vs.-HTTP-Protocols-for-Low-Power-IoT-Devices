# Performance Analysis of MQTT vs. HTTP Protocols for Low-Power IoT Devices

This project provides a benchmarking suite to compare the performance of MQTT and HTTP protocols on low-power IoT devices, specifically the ESP8266 microcontroller.

The project includes a Node.js server environment (acting as an HTTP server and an MQTT logger) and C++ firmware for the ESP8266 to run different test scenarios such as full benchmarks, stress tests, and large payload tests.

## Features

- **HTTP Server**: An Express.js server to receive HTTP POST requests from the IoT device.
- **MQTT Logger**: A Node.js script using MQTT.js to subscribe to a broker and log incoming MQTT messages.
- **ESP8266 Firmware**: Three different test scenarios developed using the Arduino framework:
  - `full_benchmark`: Standard benchmarking test.
  - `test_stress`: High-frequency stress test.
  - `test_large_payload`: Testing the protocols with larger payload sizes (e.g., 2KB JSON).

## Prerequisites

Before running the project, ensure you have the following installed:

- **Node.js** (v14 or higher) and npm
- **Arduino IDE** (or PlatformIO) to flash the ESP8266
- **Mosquitto** (or any other MQTT broker) installed and running locally or remotely

### ESP8266 Dependencies (Arduino IDE)

- `ESP8266 board package`
- `ArduinoJson` library
- `PubSubClient` library (for MQTT)
- `ESP8266HTTPClient` library

## Installation & Setup

1. **Clone the repository:**

   ```bash
   git clone <repository-url>
   cd Performance-Analysis-of-MQTT-vs.-HTTP-Protocols-for-Low-Power-IoT-Devices
   ```

2. **Install Node.js dependencies:**

   ```bash
   npm install
   ```

3. **Configure the ESP8266 Firmware:**
   - Open any of the projects inside the `microcontroller_firmware` directory in the Arduino IDE.
   - Update the Wi-Fi credentials (`ssid` and `password`).
   - Update the server/broker IP address to point to the machine where your Node.js servers and MQTT broker are running.
   - Flash the firmware to your ESP8266 board.

## Usage

### 1. Start the MQTT Broker

Ensure your MQTT broker (e.g., Mosquitto) is running. If installed locally, it usually runs on port `1883`.

### 2. Start the Servers

Open two terminal windows to run both servers concurrently:

**Start the HTTP Server:**

```bash
npm run start:http
```

**Start the MQTT Logger:**

```bash
npm run start:mqtt
```

### 3. Run the Tests

Once the servers and the MQTT broker are running, power on or reset your ESP8266. It will automatically connect to the Wi-Fi network and start executing the flashed test scenario.

The incoming data will be logged by the Node.js scripts, and any resulting data will be saved to the `research_data` directory.
