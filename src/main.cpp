#include <Arduino.h>

#include "SPInterface.h"
#include "BLEInterface.h"
#include <WiFi.h>
#include "wifi_credentials.h"

static SPInterface sinricProInterface;
static BLEInterface bleInterface;

void setupWiFi() {
  Serial.print("[Wifi]: Connecting");
  WiFi.setSleep(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\n[WiFi]: IP-Address is %d.%d.%d.%d\n", localIP[0],
                localIP[1], localIP[2], localIP[3]);
}

void setup() {
  using namespace std::placeholders;

  Serial.begin(115200);
  Serial.println("Setting up WiFI...");
  setupWiFi();
  
  sinricProInterface.setPowerStateCallback(std::bind(&BLEInterface::setPowerState, &bleInterface, _1));
  sinricProInterface.setBrightnessCallback(std::bind(&BLEInterface::setBrightness, &bleInterface, _1));

  Serial.println("Setting up Sinric Pro...");
  sinricProInterface.init();
  Serial.println("Setting up BLE...");
  bleInterface.init();
}

void loop() {
  bleInterface.loop();
  sinricProInterface.loop();
}
