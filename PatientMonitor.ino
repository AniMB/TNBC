#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include "include/patient_monitor/app.h"

void setup() {
    // Logging + safe hardware states first; must return controlled status.
    patient_monitor::initialize();
}

void loop() {
    // Core already runs loop() inside a FreeRTOS task.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
