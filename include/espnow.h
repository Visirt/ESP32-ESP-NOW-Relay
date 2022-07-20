#pragma once
#include <Arduino.h>
#include <esp_now.h>

class ESPNOW
{
public:
    static void SetupESPNOW();
    static esp_err_t sendDataToRoot(String message);

    
private:
    static String macAddrString(const uint8_t * const mac_addr);
    static uint8_t* macAddrString(const String mac_addr);
    static esp_err_t addSlave(const uint8_t* const address);
    static esp_err_t sendData(String data, const uint8_t* const dest);
    static void onRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    static void onSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static bool isRoot(const uint8_t *const mac_addr);

private:
    static String currentMAC;
};