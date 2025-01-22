#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

#define LED LED_BUILTIN  

const char* ssid = "Smol";
const char* password = "flatrone2360";
uint16_t port = 8765;

WebSocketsClient WebSocket;
//JsonDocument doc;
StaticJsonDocument<200> doc;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
			Serial.println("Disconnected!");
			break;
    case WStype_CONNECTED: {
      Serial.println("Connected");

      doc["type"] = "name";
      doc["name"] = "espled";
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
      if (doc["led_state"] == "on") {
        digitalWrite(LED, LOW);
      }
      else if (doc["led_state"] == "off") {
        digitalWrite(LED, HIGH);
      }
      //const char* status = doc["status"];
      //const char* message = doc["message"];
      break;
    }
    default:
      break;
      
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
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
}

