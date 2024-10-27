#pragma once

#define SINRICPRO_NOSSL  // Disable SSL because of ESP memory limitations
#include <SinricPro.h>
#include <SinricProDimSwitch.h>
#include "sinric_secrets.h"

class SPInterface {
 public:
  SPInterface() = default;
  void init();
  void setPowerStateCallback(std::function<void(bool)> callback);
  void setBrightnessCallback(std::function<void(int)> callback);
  void loop();

 private:
  struct {
    bool power = false;
    int brightness = 0;
  } lightState_;

  bool onPowerState(const String &deviceId, bool &state);
  bool onBrightness(const String &deviceId, int &brightness);
  bool onAdjustBrightness(const String &deviceId, int brightnessDelta);

  std::function<void(bool)> powerStateCallback_;
  std::function<void(int)> brightnessCallback_;
};


void SPInterface::init() {
  using namespace std::placeholders;
  // Het a new Light device from SinricPro
  SinricProDimSwitch &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(std::bind(&SPInterface::onPowerState, this, _1, _2));
  myLight.onPowerLevel(std::bind(&SPInterface::onBrightness, this,  _1, _2));
  myLight.onAdjustPowerLevel(
      std::bind(&SPInterface::onAdjustBrightness, this, _1, _2));

  // setup SinricPro
  SinricPro.onConnected([]() { Serial.println("Connected to SinricPro"); });
  SinricPro.onDisconnected(
      []() { Serial.println("Disconnected from SinricPro"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void SPInterface::setPowerStateCallback(std::function<void(bool)> callback) {
  powerStateCallback_ = callback;
}

void SPInterface::setBrightnessCallback(std::function<void(int)> callback) {
  brightnessCallback_ = callback;
}

bool SPInterface::onPowerState(const String &deviceId, bool &state) {
  lightState_.power = state;
  powerStateCallback_(lightState_.power);
  Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(),
                state ? "on" : "off");
  return true;
}

bool SPInterface::onBrightness(const String &deviceId, int &brightness) {
  lightState_.brightness = brightness;
  if (lightState_.brightness > 100) {
    lightState_.brightness = 100;
  } else if (lightState_.brightness < 1) {
    lightState_.brightness = 1;
  }
  brightnessCallback_(lightState_.brightness);
  Serial.printf("Device %s brightness level changed to %d\n",
                deviceId.c_str(), brightness);
  return true;
}

bool SPInterface::onAdjustBrightness(const String &deviceId,
                                         int brightnessDelta) {
  lightState_.brightness += brightnessDelta;
  if (lightState_.brightness > 100) {
    lightState_.brightness = 100;
  } else if (lightState_.brightness < 1) {
    lightState_.brightness = 1;
  }
  brightnessCallback_(lightState_.brightness);
  Serial.printf("Device %s brightness level changed by %d\nNew value: %d",
                deviceId.c_str(), brightnessDelta, lightState_.brightness);
  return true;
}

void SPInterface::loop() { SinricPro.handle(); }