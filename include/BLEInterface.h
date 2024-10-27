#pragma once

#include <Arduino.h>
#include <BLEDevice.h>

class BLEInterface {
 public:
  BLEInterface();
  void init();
  void setPowerStateCallback(std::function<bool(bool)> callback);
  void setBrightnessCallback(std::function<bool(int)> callback);
  bool writePowerState(bool state);
  bool writeBrightness(int brightness);
  void loop();

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

  void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                             uint8_t *pData, size_t length, bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((String)(*pData));
    if (length != 1) {
      return;
    }
    if (pBLERemoteCharacteristic == pPowerCharacteristic) {
      powerStateCallback_(*pData);
    } else if (pBLERemoteCharacteristic == pBrightnessCharacteristic) {
      int brightness = map(*pData, 1, 254, 1, 100);
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
};

  //   class MyClientCallback : public BLEClientCallbacks {
  //     void onConnect(BLEClient *pclient) {}

  //     void onDisconnect(BLEClient *pclient) {
  //       connected = false;
  //       Serial.println("BLE Disconnected");
  //     }
  //   };
  // };