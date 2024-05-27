
#include "NimBLEDevice.h"
#include "NimBLELog.h"

#include <stdio.h>

extern "C" {
    void app_main(void);
}

static NimBLEServer* pServer;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        printf("\nClient address: %s\n", connInfo.getAddress().toString().c_str());
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 18);
    };

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        printf("Client disconnected - start advertising\n\n");
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) {
        printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
    };

    uint32_t onPassKeyRequest(){
        printf("Server Passkey Request\n");
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key){
        printf("The passkey YES/NO number: %" PRIu32"\n", pass_key);
        return true;
    };

    void onAuthenticationComplete(NimBLEConnInfo& connInfo){
        printf("--- onAuthenticationComplete ---\n");
        printf("Authenticated: %s\n", connInfo.isAuthenticated()   ?"Yes":"No");
        printf("Encrypted:     %s\n", connInfo.isEncrypted()       ?"Yes":"No");
        printf("Bonded:        %s\n", connInfo.isBonded()          ?"Yes":"No");
    };
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        printf("%s : onRead(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        printf("%s : onWrite(), value: %s\n",
               pCharacteristic->getUUID().toString().c_str(),
               pCharacteristic->getValue().c_str());
    }

    void onNotify(NimBLECharacteristic* pCharacteristic) {
        printf("Sending notification to clients\n");
    }

    void onStatus(NimBLECharacteristic* pCharacteristic, int code) {
        printf("Notification/Indication return code: %d, %s\n",
               code, NimBLEUtils::returnCodeToString(code));
    }

    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        std::string str = "Client ID: ";
        str += connInfo.getConnHandle();
        str += " Address: ";
        str += connInfo.getAddress().toString();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());

        printf("%s\n", str.c_str());
    }
};

class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
        std::string dscVal = pDescriptor->getValue();
        printf("Descriptor witten value: %s\n", dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) {
        printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
    };;
};

void notifyTask(void * parameter){
    for(;;) {
        if(pServer->getConnectedCount()) {
            NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
            if(pSvc) {
                NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
                if(pChr) {
                    pChr->notify(true);
                }
            }
        }
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

void app_main(void) {
    printf("Starting NimBLE Server\n");

    NimBLEDevice::init("NimBLE");
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    // (BLE_SM_PAIR_AUTHREQ_BOND, BLE_SM_PAIR_AUTHREQ_MITM, BLE_SM_PAIR_AUTHREQ_SC)
    NimBLEDevice::setSecurityAuth(true, false, false); 

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pDeadService = pServer->createService("DEAD");
    NimBLECharacteristic* pBeefCharacteristic = pDeadService->createCharacteristic(
                                               "BEEF",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::READ_ENC | 
                                               NIMBLE_PROPERTY::WRITE_ENC
                                              );

    pBeefCharacteristic->setValue("Burger");
    pBeefCharacteristic->setCallbacks(&chrCallbacks);


    NimBLE2904* pBeef2904 = (NimBLE2904*)pBeefCharacteristic->createDescriptor("2904");
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pBeef2904->setCallbacks(&dscCallbacks);

    NimBLEService* pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic = pBaadService->createCharacteristic(
                                               "F00D",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    pFoodCharacteristic->setValue("Fries");
    pFoodCharacteristic->setCallbacks(&chrCallbacks);

    NimBLEDescriptor* pC01Ddsc = pFoodCharacteristic->createDescriptor(
                                               "C01D",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE|
                                               NIMBLE_PROPERTY::WRITE_ENC,
                                               20
                                              );
    pC01Ddsc->setValue("Send it back!");
    pC01Ddsc->setCallbacks(&dscCallbacks);

    pDeadService->start();
    pBaadService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

    pAdvertising->addServiceUUID(pDeadService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());

    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    printf("Advertising Started\n");

    xTaskCreate(notifyTask, "notifyTask", 5000, NULL, 1, NULL);
}
