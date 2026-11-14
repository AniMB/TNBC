#include "../include/patient_monitor/app.h"
#include "../include/patient_monitor/spi_bus.h"
#include "../include/patient_monitor/bp.h"
#include "../include/patient_monitor/ecg_task.h"
#include "../include/patient_monitor/spo2_task.h"
#include "../include/patient_monitor/ble_task.h"
#include "../include/patient_monitor/display_task.h"
#include "../include/patient_monitor/status_task.h"
#include "../include/patient_monitor/power_task.h"
#include "../include/patient_monitor/sd_task.h"
#include "../include/patient_monitor/profile_map.h"
#include "../config/defaults.h"

#if defined(ARDUINO) || defined(__arm__)
#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#endif

namespace patient_monitor {

static MockSpiBus s_spi_bus;
static MockBpHardware s_bp_hardware;

static EcgRingBuffer s_ecg_ring_buffer(config::ECG_RING_BUFFER_CAPACITY);
static EcgTask s_ecg_task(s_spi_bus, s_ecg_ring_buffer);
static Spo2Task s_spo2_task;
static BleTask s_ble_task;
static DisplayTask s_display_task(s_spi_bus);
static StatusTask s_status_task;
static PowerTask s_power_task;

class MockSdStorageBackend : public ISdStorageBackend {
public:
    bool isCardPresent() override { return true; }
    bool openSessionDirectory(uint32_t /*patient_id*/, uint32_t /*session_id*/) override { return true; }
    bool appendRecord(StreamType /*stream*/, const uint8_t* /*data*/, size_t /*length*/) override { return true; }
    bool sync() override { return true; }
    size_t getCurrentFileOffset(StreamType /*stream*/) override { return 0; }
};

static MockSdStorageBackend s_mock_sd_backend;
static SdTask s_sd_task(s_mock_sd_backend);

ISpiBus& getSharedSpiBus() {
    return s_spi_bus;
}

#if defined(ARDUINO) || defined(__arm__)
static TaskHandle_t s_handle_bp_worker   = nullptr;
static TaskHandle_t s_handle_ecg_task    = nullptr;
static TaskHandle_t s_handle_spo2_task   = nullptr;
static TaskHandle_t s_handle_status_task = nullptr;
static TaskHandle_t s_handle_power_task  = nullptr;
static TaskHandle_t s_handle_ble_task    = nullptr;
static TaskHandle_t s_handle_display_task= nullptr;
static TaskHandle_t s_handle_sd_task     = nullptr;

static void ecgTaskWrapper(void* param) {
    (void)param;
    while (true) {
        s_ecg_task.step();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

static void spo2TaskWrapper(void* param) {
    (void)param;
    while (true) {
        s_spo2_task.step();
        vTaskDelay(pdMS_TO_TICKS(config::SPO2_SAMPLE_INTERVAL_MS));
    }
}

static void bleTaskWrapper(void* param) {
    (void)param;
    while (true) {
        s_ble_task.step();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void displayTaskWrapper(void* param) {
    (void)param;
    while (true) {
        bool bp_active = getModeCoordinator().isBpExclusive();
        s_display_task.step(s_status_task.getSnapshot(), bp_active);
        vTaskDelay(pdMS_TO_TICKS(config::DISPLAY_REFRESH_INTERVAL_MS));
    }
}

static void statusTaskWrapper(void* param) {
    (void)param;
    while (true) {
        s_status_task.step();
        vTaskDelay(pdMS_TO_TICKS(config::STATUS_UPDATE_INTERVAL_MS));
    }
}

static void powerTaskWrapper(void* param) {
    (void)param;
    while (true) {
        s_power_task.step();
        s_status_task.updateBatteryMetric(s_power_task.getBatteryMillivolts(), s_power_task.getBatteryPercent());
        vTaskDelay(pdMS_TO_TICKS(config::POWER_POLL_INTERVAL_MS));
    }
}

static void sdTaskWrapper(void* param) {
    (void)param;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void bpWorkerWrapper(void* param) {
    (void)param;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
#endif

bool initialize() {
    getBpHardware().emergencyDeflate();

    if (config::SCHEMA_VERSION == 0) {
        getModeCoordinator().tripError("Invalid schema version");
        return false;
    }

    s_spi_bus.unlock();

    if (!s_sd_task.switchSession(config::DEFAULT_PATIENT_ID, config::DEFAULT_SESSION_ID)) {
        getModeCoordinator().tripError("Session initialization failed");
        return false;
    }

#if defined(ARDUINO) || defined(__arm__)
    BaseType_t res;

    res = xTaskCreate(bpWorkerWrapper, "BP_Worker", config::STACK_BP_WORKER, nullptr,
                      config::PRIORITY_BP_WORKER, &s_handle_bp_worker);
    if (res != pdPASS || s_handle_bp_worker == nullptr) return false;

    res = xTaskCreate(ecgTaskWrapper, "ECG", config::STACK_ECG_TASK, nullptr,
                      config::PRIORITY_ECG_TASK, &s_handle_ecg_task);
    if (res != pdPASS || s_handle_ecg_task == nullptr) return false;

    res = xTaskCreate(spo2TaskWrapper, "SpO2", config::STACK_SPO2_TASK, nullptr,
                      config::PRIORITY_SPO2_TASK, &s_handle_spo2_task);
    if (res != pdPASS || s_handle_spo2_task == nullptr) return false;

    res = xTaskCreate(statusTaskWrapper, "Status", config::STACK_STATUS_TASK, nullptr,
                      config::PRIORITY_STATUS_TASK, &s_handle_status_task);
    if (res != pdPASS || s_handle_status_task == nullptr) return false;

    res = xTaskCreate(powerTaskWrapper, "Power", config::STACK_POWER_TASK, nullptr,
                      config::PRIORITY_POWER_TASK, &s_handle_power_task);
    if (res != pdPASS || s_handle_power_task == nullptr) return false;

    res = xTaskCreate(bleTaskWrapper, "BLE", config::STACK_BLE_TASK, nullptr,
                      config::PRIORITY_BLE_TASK, &s_handle_ble_task);
    if (res != pdPASS || s_handle_ble_task == nullptr) return false;

    res = xTaskCreate(displayTaskWrapper, "Display", config::STACK_DISPLAY_TASK, nullptr,
                      config::PRIORITY_DISPLAY_TASK, &s_handle_display_task);
    if (res != pdPASS || s_handle_display_task == nullptr) return false;

    res = xTaskCreate(sdTaskWrapper, "SD_Storage", config::STACK_SD_TASK, nullptr,
                      config::PRIORITY_SD_TASK, &s_handle_sd_task);
    if (res != pdPASS || s_handle_sd_task == nullptr) return false;
#endif

    getModeCoordinator().requestMode(SystemMode::IDLE);
    return true;
}

void handleProfileButtonIsr() {
}

void handleEcgDrdyIsr() {
    s_ecg_task.onDataReady();
}

} // namespace patient_monitor
