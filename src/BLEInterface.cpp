#include "BLEInterface.h"

BLEInterface::BLEInterface()
    : connectServiceUUID("0000fe0f-0000-1000-8000-00805f9b34fb"),
      controlServiceUUID("932c32bd-0000-47a2-835a-a8d455b859dd"),
      bindCharUUID("0000fe0f-0000-1000-8000-00805f9b34fb"),
      powerCharUUID("932c32bd-0002-47a2-835a-a8d455b859dd"),
      brightnessCharUUID("932c32bd-0003-47a2-835a-a8d455b859dd") {}

void BLEInterface::init() {
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when
  // we have detected a new device.  Specify that we want active scanning and
  // start the scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(
      [this](BLEAdvertisedDevice advertisedDevice) {
        // Scan for BLE servers and find the first one that advertises the
        // service we are looking for.
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        if (advertisedDevice.haveServiceUUID() &&
            (advertisedDevice.isAdvertisingService(connectServiceUUID) ||
             advertisedDevice.isAdvertisingService(controlServiceUUID))) {
          BLEDevice::getScan()->stop();
          device = new BLEAdvertisedDevice(advertisedDevice);
          Serial.println("Found target device! Connecting...");
          doConnect = true;
        }
      }));

  pBLEScan->setInterval(189);
  pBLEScan->setWindow(49);
  pBLEScan->setActiveScan(true);
  Serial.println("STARTING BLE SCAN");
  pBLEScan->start(5, false);
}

void BLEInterface::setPowerStateCallback(std::function<bool(bool)> callback) {
  powerStateCallback_ = callback;
}

void BLEInterface::setBrightnessCallback(std::function<bool(int)> callback) {
  brightnessCallback_ = callback;
}

bool BLEInterface::writePowerState(bool state) {
  if (pPowerCharacteristic == nullptr) {
    Serial.println("Error: pPowerCharacteristic is nullptr");
    return false;
  }
  uint8_t value = state ? 0x01 : 0x00;
  pPowerCharacteristic->writeValue(value);
  return true;
}

bool BLEInterface::writeBrightness(int brightness) {
  if (pBrightnessCharacteristic == nullptr) {
    Serial.println("Error: pBrightnessCharacteristic is nullptr");
    return false;
  }
  uint8_t value = map(brightness, 1, 100, 1, 254);
  Serial.println("Setting brightness to " + String(value));
  pBrightnessCharacteristic->writeValue(value);
  return true;
}

bool BLEInterface::connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(device->getAddress().toString().c_str());

  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  // pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote BLE Server.
  pClient->connect(device);  // if you pass BLEAdvertisedDevice instead of
                             // address, it will be recognized type of peer
                             // device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  // set client to request maximum MTU from server
                         // (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(controlServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(controlServiceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE
  // server.
  pPowerCharacteristic = pRemoteService->getCharacteristic(powerCharUUID);
  if (pPowerCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(powerCharUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  pBrightnessCharacteristic =
      pRemoteService->getCharacteristic(brightnessCharUUID);
  if (pBrightnessCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(brightnessCharUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found our characteristics");

  // Read the value of the characteristic.
  if (pPowerCharacteristic->canRead()) {
    std::string value = pPowerCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  using namespace std::placeholders;

  if (pPowerCharacteristic->canNotify()) {
    Serial.println("Power charactristic can notify");
    pPowerCharacteristic->registerForNotify(std::bind(&BLEInterface::notifyCallback, this, _1, _2, _3, _4));
  }

  if (pBrightnessCharacteristic->canNotify()) {
    Serial.println("Brightness charactristic can notify");
    pBrightnessCharacteristic->registerForNotify(std::bind(&BLEInterface::notifyCallback, this, _1, _2, _3, _4));
  }

  connected = true;
  return true;
}

void BLEInterface::loop() {
  if (doConnect) {
    connected = connectToServer();
    Serial.println(connected ? "We are now connected to the BLE Server."
                             : "We have failed to connect to the server; there "
                               "is nothin more we will do.");
    doConnect = false;
  }
}