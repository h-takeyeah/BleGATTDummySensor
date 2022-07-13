#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define SERVICE_UUID 0x1801 // Generic Attribute
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #define SERVICE_UUID        "0000ffe0-0000-1000-8000-00805f9b34fb"
// #define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

// #define SERVICE_UUID 0xffe0 // atorch service
// #define CHARACTERISTIC_UUID 0xffe1 // atorch characteristic

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
    {
        log_i("connected. addr: %s", BLEAddress(param->connect.remote_bda).toString().c_str());
        deviceConnected = true;
    }

    void onDisconnect(BLEServer *pServer)
    {
        log_i("disconnected");
        deviceConnected = false;
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param)
    {
        log_d(">> onRead: custom");
        log_d("gatt evt type: %u", param);
        log_d("<< onRead");
    }

    void onNotify(BLECharacteristic *pCharacteristic)
    {
        log_d(">> onNotify: custom");
        log_d("<< onNotify");
    }

    void onStatus(BLECharacteristic *pCharacteristic, Status s, uint32_t code)
    {
        log_d(">> onStatus: custom");
        log_d("characteristic: %s, status: %u, code: %u", pCharacteristic->toString().c_str(), s, code);
        log_d("<< onStatus");
    }
};

void setup()
{
    M5.begin(false, false);
    M5.Power.begin();

    // Create the BLE Device
    BLEDevice::init("M5Stack");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    // BLE2902 *pDescriptor = new BLE2902();
    // pDescriptor->setNotifications(true); // enables notification and also confirmation
    pCharacteristic->addDescriptor(new BLE2902());
    // pCharacteristic->addDescriptor(pDescriptor);

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter

    // -- not working, need help --
    // BLEAdvertisementData advert;
    // advert.setServiceData(BLEUUID(SERVICE_UUID), "testtest");
    // advert.setName("M5Stack GREY");
    // advert.setShortName("M5Stack");
    // pAdvertising->setAdvertisementData(advert);
    // -- not working --

    BLEDevice::startAdvertising();
    log_i("Waiting a client connection to notify...");
}

void loop()
{
    // notify changed value
    if (deviceConnected)
    {
        pCharacteristic->setValue((uint8_t *)&value, 4);
        // pCharacteristic->setValue("FF5501030001F3000000000638000003110007000A000000122E333C000000000000004E");
        pCharacteristic->notify();
        value++;
        delay(5000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500); // give the bluetooth stack the chance to get things ready
        // pServer->startAdvertising(); // restart advertising
        BLEDevice::startAdvertising(); // restart advertising
        log_i("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
