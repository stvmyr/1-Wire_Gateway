#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "credentials.h"

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

AsyncWebServer server(80);

String getSensorAddressString(DeviceAddress address) {
  String addressString = "";
  for (uint8_t i = 0; i < 8; i++) {
    if (addressString.length() > 0) {
      addressString += ":";
    }
    if (address[i] < 16) {
      addressString += "0";
    }
    addressString += String(address[i], HEX);
  }
  return addressString;
}

void handleNotFound(AsyncWebServerRequest* request) {
  String sensorAddress = request->url();
  sensorAddress.remove(0, 1);
  
  DeviceAddress addr;
  int sensorCount = sensors.getDeviceCount();
  for (int i = 0; i < sensorCount; i++) {
    if (sensors.getAddress(addr, i)) {
      if (sensorAddress.equalsIgnoreCase(getSensorAddressString(addr))) {
        sensors.requestTemperaturesByAddress(addr);
        float temperature = sensors.getTempCByIndex(i);
        request->send(200, "text/plain", String(temperature));
        return;
      }
    }
  }

  request->send(404);
}

void handleListAddresses(AsyncWebServerRequest* request) {
  String addressList = "";

  sensors.begin();  // Scan for new sensors

  DeviceAddress addr;
  int sensorCount = sensors.getDeviceCount();
  for (int i = 0; i < sensorCount; i++) {
    if (sensors.getAddress(addr, i)) {
      addressList += getSensorAddressString(addr);
      addressList += "\n";
    }
  }

  request->send(200, "text/plain", addressList);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  sensors.begin();

  server.onNotFound(handleNotFound);
  server.on("/list", HTTP_GET, handleListAddresses);

  server.begin();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost Wi-Fi connection. Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Attempting to reconnect to Wi-Fi...");
    }
    Serial.println("Wi-Fi reconnected.");
  }
}
