//Include Required Libraries
#include <WiFi.h>
#include "DHT.h"
#include <BlynkSimpleEsp32.h>

// Sensor and Pin Definitions
#define MQ3_PIN 34           // Analog pin for the MQ3 sensor
#define DHT_PIN 15           // Digital pin for the DHT11 sensor
#define DHT_TYPE DHT11       // Specify the type of DHT sensor (DHT11)
#define RAIN_SENSOR_PIN 35   // Analog pin for the rain sensor
#define LDR_PIN 32           // Analog pin for the LDR sensor

// Wi-Fi credentials
const char* ssid     = "WIFI NAME";
const char* password = "Password";

// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL3jcrPQ-pL"
#define BLYNK_TEMPLATE_NAME "sample"
#define BLYNK_AUTH_TOKEN "mkB-3gNMQgRv5q75mDsUlzvSV1WyedWT"
//Object Initialization
WiFiServer server(80);
DHT dht(DHT_PIN, DHT_TYPE);

// Lighting thresholds
const int nightThreshold = 500;
const int sunnyThreshold = 3000;
const int cloudyThreshold = 1000;

void setup() {
    Serial.begin(115200);
    pinMode(MQ3_PIN, INPUT);
    pinMode(RAIN_SENSOR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    dht.begin();

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();

    // Initialize Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

int readMQ3() {
    return analogRead(MQ3_PIN);
}

int readRainSensor() {
    return analogRead(RAIN_SENSOR_PIN);
}

int readLDR() {
    return analogRead(LDR_PIN);
}

String getLightingCondition(int ldrValue) {
    if (ldrValue < nightThreshold) {
        return "Night";
    } else if (ldrValue > sunnyThreshold) {
        return "Sunny";
    } else if (ldrValue < cloudyThreshold) {
        return "Cloudy";
    } else {
        return "Partially Cloudy";
    }
}

void loop() {
    Blynk.run();
    int mq3Value = readMQ3();
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int rainValue = readRainSensor();
    int ldrValue = readLDR();
    String lightingCondition = getLightingCondition(ldrValue);

    // Check for DHT sensor errors
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Print readings to Serial Monitor
    Serial.print("MQ3 Gas Level: ");
    Serial.println(mq3Value);
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Rain Sensor Value: ");
    Serial.println(rainValue);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);
    Serial.print("Lighting Condition: ");
    Serial.println(lightingCondition);

    // Update Blynk
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, mq3Value);
    Blynk.virtualWrite(V3, rainValue);
    Blynk.virtualWrite(V4, lightingCondition);

    // Serve the web page
    WiFiClient client = server.available();
    if (client) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println("Refresh: 10");
        client.println();

        client.println("<!DOCTYPE HTML>");
        client.println("<html><head><title>ESP32 Weather Report</title>");
        client.println("<style>");
        // client.println("body { background: url('https://indoafrica.allegiance-educare.in/storage/uploads/colleges/1417413747Capture1.JPG') no-repeat center center fixed; background-size: cover; color: white; font-family: Arial, sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }");
        client.println("h1 { font-size: 2em; text-align: center; }");
        client.println("p { font-size: 1.2em; margin: 10px; text-align: center; }");
        client.println("</style></head><body>");
        client.println("<h1>ESP32 Weather Report</h1>");
        client.println("<p>MQ3 Gas Level: " + String(mq3Value) + "</p>");
        client.println("<p>Humidity: " + String(humidity) + " %</p>");
        client.println("<p>Temperature: " + String(temperature) + " °C</p>");
        client.println("<p>Rain Sensor Value: " + String(rainValue) + "</p>");
        client.println("<p>LDR Value: " + String(ldrValue) + "</p>");
        client.println("<p>Lighting Condition: " + lightingCondition + "</p>");
        client.println("</body></html>");

        client.stop();  // Close the connection
    }
}
