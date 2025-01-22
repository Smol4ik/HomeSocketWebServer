#include <Arduino.h>
#include "DHT.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

#define DHTPIN 2  
#define DHTTYPE DHT11

const char* ssid = "Smol";
const char* password = "flatrone2360";
uint16_t port = 8765;

unsigned long lastTime = 0;
//unsigned long timerDelay = 2000;

WebSocketsClient WebSocket;
//JsonDocument doc;
StaticJsonDocument<200> doc;
DHT dht(DHTPIN, DHTTYPE);

void sendData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
    //if(WiFi.status()== WL_CONNECTED){
      if (isnan(humidity) || isnan(temperature)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }
      doc["type"] = "data_information";
      doc["selected_client"] = "frontend";
      JsonObject data = doc.createNestedObject("data");
        //JsonObject data = doc["data"].to<JsonObject>();
      data["temperature"] = temperature;
      data["humidity"] = humidity;
      String output;
      serializeJson(doc, output);
      WebSocket.sendTXT(output);
    }
//}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
			Serial.println("Disconnected!");
			break;
    case WStype_CONNECTED: {
      Serial.println("Connected");

      doc["type"] = "name";
      doc["name"] = "esptemp";
      String output;
      serializeJson(doc, output);
			WebSocket.sendTXT(output);
      break;
    }
      
    case WStype_TEXT: {
      Serial.printf("[WSc] get text: %s\n", payload);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      const char* status = doc["status"];
      const char* message = doc["message"];
      break;
    }
    default:
      break;
      
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(500);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  WebSocket.begin("192.168.0.103", port);
  WebSocket.onEvent(webSocketEvent);
}

void loop() {
  WebSocket.loop();
  if((millis() - lastTime) > 5000) {
    sendData();
    lastTime = millis();
  }
  
}
/*
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebSocketsClient WebSocket;
const char* ssid = "Smol";
const char* password = "flatrone2360";
uint16_t port = 8765;

void sendData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  StaticJsonDocument<200> doc;
  doc["type"] = "data_information";
  doc["selected_client"] = "frontend";

  JsonObject data = doc.createNestedObject("data");
  data["temperature"] = temperature;
  data["humidity"] = humidity;

  String output;
  serializeJson(doc, output);
  WebSocket.sendTXT(output);
  Serial.println("Data sent: " + output);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("WebSocket connected");
  } else if (type == WStype_DISCONNECTED) {
    Serial.println("WebSocket disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  WebSocket.begin("192.168.0.103", port);
  WebSocket.onEvent(webSocketEvent);
}

void loop() {
  WebSocket.loop();
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 2000) {
    sendData();
    lastTime = millis();
  }
}*/
