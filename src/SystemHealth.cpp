#include "SystemHealth.h"

#include <Arduino.h>
#include "Config.h"

#if defined(ESP32)
#include <esp_err.h>
#include <esp_idf_version.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#endif

void SystemHealth::begin() {
#if defined(ESP32)
  _resetReason = static_cast<uint32_t>(esp_reset_reason());

  esp_err_t result = ESP_FAIL;
#if ESP_IDF_VERSION_MAJOR >= 5
  esp_task_wdt_config_t config = {};
  config.timeout_ms = SYSTEM_TASK_WATCHDOG_TIMEOUT_MS;
  config.idle_core_mask = 0;
  config.trigger_panic = true;
  result = esp_task_wdt_init(&config);
#else
  result = esp_task_wdt_init((SYSTEM_TASK_WATCHDOG_TIMEOUT_MS + 999) / 1000, true);
#endif

  if (result == ESP_OK || result == ESP_ERR_INVALID_STATE) {
    const esp_err_t addResult = esp_task_wdt_add(nullptr);
    _watchdogActive = addResult == ESP_OK || addResult == ESP_ERR_INVALID_ARG;
  }
#endif
}

void SystemHealth::feed() {
#if defined(ESP32)
  if (_watchdogActive) {
    esp_task_wdt_reset();
  }
#endif
}

bool SystemHealth::watchdogActive() const {
  return _watchdogActive;
}

uint32_t SystemHealth::resetReason() const {
  return _resetReason;
}

const char* SystemHealth::resetReasonName() const {
#if defined(ESP32)
  switch (static_cast<esp_reset_reason_t>(_resetReason)) {
    case ESP_RST_POWERON: return "power_on";
    case ESP_RST_EXT: return "external";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt_watchdog";
    case ESP_RST_TASK_WDT: return "task_watchdog";
    case ESP_RST_WDT: return "other_watchdog";
    case ESP_RST_DEEPSLEEP: return "deep_sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unknown";
  }
#else
  return "unsupported";
#endif
}
