#pragma once

#include <Arduino.h>
#include <BLEDevice.h>

class BLEInterface {
 public:
  BLEInterface();
  void init();
  bool setPowerState(bool state);
  bool setBrightness(int brightness);
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
  static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                             uint8_t *pData, size_t length, bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((String)(*pData));
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