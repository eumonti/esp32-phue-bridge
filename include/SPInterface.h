#pragma once

#define SINRICPRO_NOSSL  // Disable SSL because of ESP memory limitations
#include <SinricPro.h>
#include <SinricProDimSwitch.h>

#include "sinric_secrets.h"

class SPInterface {
 public:
  SPInterface() = default;
  void init();
  void setPowerStateCallback(std::function<bool(bool)> callback);
  void setBrightnessCallback(std::function<bool(int)> callback);
  bool updatePowerState(bool state);
  bool updateBrightness(int brightness);
  void updateRemoteStateDebounce();
  void loop();

 private:
  struct {
    bool power = false;
    int brightness = 0;
  } lightState_;

  bool onPowerState(const String &deviceId, bool &state);
  bool onBrightness(const String &deviceId, int &brightness);
  bool onAdjustBrightness(const String &deviceId, int brightnessDelta);

  unsigned long lastPowerStateEvent = 0;
  bool shouldWritePowerState = false;
  unsigned long lastBrightnessEvent = 0;
  bool shouldWriteBrightness = false;
  unsigned long lastRequest = 0;

  std::function<bool(bool)> powerStateCallback_;
  std::function<bool(int)> brightnessCallback_;
};

void SPInterface::init() {
  using namespace std::placeholders;
  // Het a new Light device from SinricPro
  SinricProDimSwitch &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(std::bind(&SPInterface::onPowerState, this, _1, _2));
  myLight.onPowerLevel(std::bind(&SPInterface::onBrightness, this, _1, _2));
  myLight.onAdjustPowerLevel(
      std::bind(&SPInterface::onAdjustBrightness, this, _1, _2));

  // setup SinricPro
  SinricPro.onConnected([]() { Serial.println("Connected to SinricPro"); });
  SinricPro.onDisconnected(
      []() { Serial.println("Disconnected from SinricPro"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void SPInterface::setPowerStateCallback(std::function<bool(bool)> callback) {
  powerStateCallback_ = callback;
}

void SPInterface::setBrightnessCallback(std::function<bool(int)> callback) {
  brightnessCallback_ = callback;
}

bool SPInterface::updatePowerState(bool state) {
  lightState_.power = state;
  shouldWritePowerState = true;
  lastPowerStateEvent = millis();
  return true;
}

bool SPInterface::updateBrightness(int brightness) {
  lightState_.brightness = brightness;
  shouldWriteBrightness = lightState_.power; // Only write brightness if the light is on to avoid turning it back on
  lastBrightnessEvent = millis();
  return true;
}

bool SPInterface::onPowerState(const String &deviceId, bool &state) {
  lightState_.power = state;
  return powerStateCallback_(lightState_.power);
}

bool SPInterface::onBrightness(const String &deviceId, int &brightness) {
  lightState_.brightness = brightness;
  if (lightState_.brightness > 100) {
    lightState_.brightness = 100;
  } else if (lightState_.brightness < 1) {
    lightState_.brightness = 1;
  }
  return brightnessCallback_(lightState_.brightness);
}

bool SPInterface::onAdjustBrightness(const String &deviceId,
                                     int brightnessDelta) {
  lightState_.brightness += brightnessDelta;
  if (lightState_.brightness > 100) {
    lightState_.brightness = 100;
  } else if (lightState_.brightness < 1) {
    lightState_.brightness = 1;
  }
  return brightnessCallback_(lightState_.brightness);
}

void SPInterface::updateRemoteStateDebounce() {
  if (millis() - lastRequest <= 3000) { // At least 3 seconds between requests
    return;
  }

  if (millis() - lastBrightnessEvent > 3000 && shouldWriteBrightness) { // Wait 3 seconds to be sure the state is stable
    SinricProDimSwitch &myLight = SinricPro[LIGHT_ID];
    myLight.sendPowerLevelEvent(lightState_.brightness);
    lastRequest = millis();
    shouldWriteBrightness = false;
    return;
  }

  if (millis() - lastPowerStateEvent > 3000 && shouldWritePowerState) { // Wait 3 seconds to be sure the state is stable
    SinricProDimSwitch &myLight = SinricPro[LIGHT_ID];
    myLight.sendPowerStateEvent(lightState_.power);
    lastRequest = millis();
    shouldWritePowerState = false;
    return;
  }
}

void SPInterface::loop() {
  updateRemoteStateDebounce();
  SinricPro.handle();
}