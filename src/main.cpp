#include <constants_global.h>
#include <espnow.h>
#include <esp_task_wdt.h>

TickType_t taskWaitTime;

void heartbeatTask(TimerHandle_t handle){
  #ifdef DEBUG
    Serial.println("Executing Heartbeat");
  #endif
  ESPNOW::sendDataToRoot("TESTING");
}

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  ESPNOW::SetupESPNOW();

  esp_task_wdt_init(2, true);
  esp_task_wdt_add(NULL);
  xTimerStart(xTimerCreate("heartbeatTimer", pdMS_TO_TICKS(10000), pdTRUE, (void*)0, heartbeatTask), portMAX_DELAY);
  taskWaitTime = xTaskGetTickCount();
  heartbeatTask(NULL);
}

void loop() {
  esp_task_wdt_reset();
  vTaskDelayUntil(&taskWaitTime, pdMS_TO_TICKS(200));
}