#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include "utils.h"

class BLEInterface {
 public:
  BLEInterface();
  void init();
  void setPowerStateCallback(std::function<bool(bool)> callback);
  void setBrightnessCallback(std::function<bool(int)> callback);
  bool writePowerState(bool state);
  bool writeBrightness(int brightness);
  void loop();
  bool readPowerState();
  int readBrightness();

 private:
  // BLE service UUIDs
  BLEUUID connectServiceUUID;
  BLEUUID controlServiceUUID;
  // BLE service characteristics UUIDs
  BLEUUID bindCharUUID;
  BLEUUID powerCharUUID;
  BLEUUID brightnessCharUUID;

  //! BLE device
  BLEAdvertisedDevice *device;

  //! BLE service characteristics
  BLERemoteCharacteristic *pPowerCharacteristic;
  BLERemoteCharacteristic *pBrightnessCharacteristic;

  bool connected = false;
  bool doConnect = false;

  bool connectToServer();
  std::function<bool(bool)> powerStateCallback_;
  std::function<bool(int)> brightnessCallback_;

  /**
   * @brief Callback for notify events.
   * @param pBLERemoteCharacteristic
   * @param pData
   * @param length
   * @param isNotify true if this is a notify, false if it is an indicate.
   */

  void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                      uint8_t *pData, size_t length, bool isNotify) {
    if (length != 1) {
      return;
    }
    if (pBLERemoteCharacteristic == pPowerCharacteristic) {
      Serial.println("Power state notify: " + String(*pData));
      powerStateCallback_(*pData);
    } else if (pBLERemoteCharacteristic == pBrightnessCharacteristic) {
      int brightness = brightnessByteToPercentage(*pData);
      Serial.println("Brightness notify: " + String(brightness) + "%" +
                     " (" + String(*pData) + ")");
      brightnessCallback_(brightness);
    }
  }

  class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
   public:
    MyAdvertisedDeviceCallbacks(
        std::function<void(BLEAdvertisedDevice)> &&callback)
        : callback_(std::move(callback)) {}

   private:
    //! Called for each advertising BLE server.
    std::function<void(BLEAdvertisedDevice)> callback_;
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      callback_(advertisedDevice);
    }
  };

  class MyClientCallback : public BLEClientCallbacks {
   public:
    MyClientCallback(std::function<void()> &&onConnect,
                     std::function<void()> &&onDisconnect)
        : onConnect_(std::move(onConnect)),
          onDisconnect_(std::move(onDisconnect)) {}

   private:
    std::function<void()> onConnect_;
    std::function<void()> onDisconnect_;
    void onConnect(BLEClient *pclient) { onConnect_(); }

    void onDisconnect(BLEClient *pclient) { onDisconnect_(); }
  };
};
