#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* ssid = "SMK TRUNOJOYO";
const char* password = "tanyamasoki";
const char* server = "192.168.2.6"; 
const int port = 3000;

DHT dht(2, DHT11);  // DHT sensor is connected to GPIO2

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  delay(2000);  // You can adjust the delay according to your needs
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }

  // Create a JSON object and populate it with temperature and humidity data
  StaticJsonDocument<200> jsonData;
  jsonData["temperature"] = temperature;
  jsonData["humidity"] = humidity;

  // Serialize the JSON data into a string
  String jsonPayload;
  serializeJson(jsonData, jsonPayload);

  Serial.println("JSON Data to be sent: " + jsonPayload);

  WiFiClient client;
  if (client.connect(server, port)) {
    client.println("POST /api/socket/dht HTTP/1.1");
    client.println("Host: " + String(server) + ":" + String(port));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(jsonPayload.length());
    client.println();
    client.println(jsonPayload);
    client.stop();
  } else {
    Serial.println("Connection to server failed");
  }
  delay(5000);
}
