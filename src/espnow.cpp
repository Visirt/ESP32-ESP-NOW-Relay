#include <espnow.h>
#include <ArduinoJson.h>
#include <constants_espnow.h>
#include <constants_global.h>
#include <WiFi.h>
#include <esp_wifi.h>

String ESPNOW::currentMAC;

void ESPNOW::SetupESPNOW()
{
  #ifdef DEBUG
    Serial.println("Setting up ESP_NOW");
  #endif
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();
  esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
        Serial.println("There was an error initializing ESP-NOW");
    #endif
    delay(10000);
    ESP.restart();
    return;
  }
  else
  {
    #ifdef DEBUG
      Serial.println("ESP-NOW Initialized");
    #endif
  }
  
  ESPNOW::currentMAC = WiFi.macAddress();

  if (addSlave(ROOT_NODE) != ESP_OK)
    ESP.restart();
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onRecv);
}

void ESPNOW::onRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) {
  String data = String((char*)incomingData).substring(0, len);
  if(isRoot(mac_addr))
  {
    DynamicJsonDocument  doc(1024);
    ArduinoJson::deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();
    String to = obj["to"];
    if(!to.isEmpty())
      sendData(data, macAddrString(to));
  }
  else
  {
    if(!esp_now_is_peer_exist(mac_addr))
      addSlave(mac_addr);
    sendData(data, ROOT_NODE);
  }
}

void ESPNOW::onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status != ESP_NOW_SEND_SUCCESS)
  {
    #ifdef DEBUG
        Serial.println("ERROR SENDING DATA");
    #endif
  }
}

String ESPNOW::macAddrString(const uint8_t * const mac_addr)
{
  return String(mac_addr[0], HEX) + ":" + String(mac_addr[1], HEX) + ":" + String(mac_addr[2], HEX) + ":" + String(mac_addr[3], HEX) + ":" + String(mac_addr[4], HEX) + ":" + String(mac_addr[5], HEX);
}

uint8_t* ESPNOW::macAddrString(const String mac_addr)
{
  if(mac_addr.length() != 13)
    return nullptr;
  uint8_t* macaddr =  new uint8_t[6];
  for(int i = 0 ; i < 6 ; i++)
  {
    macaddr[i] = (uint8_t)std::strtoul(mac_addr.substring(4 * i, 4 * i + 3).c_str(), nullptr, 16);
  }
  return macaddr;
}

esp_err_t ESPNOW::addSlave(const uint8_t* const address)
{
  esp_now_peer_info_t slaveInfo = {};
  memcpy(slaveInfo.peer_addr, address, 6);
  slaveInfo.channel = 0;
  slaveInfo.encrypt = false;
  
  return esp_now_add_peer(&slaveInfo);
}


esp_err_t ESPNOW::sendData(String data, const uint8_t* const dest)
{
  if(!esp_now_is_peer_exist(dest))
    addSlave(dest);
  #ifdef DEBUG
    Serial.println("Sending Data to:" + macAddrString(dest));
  #endif
  return esp_now_send(dest, (uint8_t*)data.c_str(), data.length());
}

bool ESPNOW::isRoot(const uint8_t *const mac_addr)
{
  const uint8_t* const root = ROOT_NODE;
  for (int i = 0; i < 6; i++)
  {
    if(mac_addr[i] != root[i])
      return false;
  }
  return true;
}

esp_err_t ESPNOW::sendDataToRoot(String message)
{
  String data = String(message);
  data.replace("\"", "\\\"");
  return sendData("{\"from\":\"" + currentMAC + "\",\"relay\":\"" + currentMAC + "\",\"message\":\"" + data + "\"}", ROOT_NODE);
}